#ifndef __BATCHFILLERSPARSETENSOR2D_CXX__
#define __BATCHFILLERSPARSETENSOR2D_CXX__

#include "BatchFillerSparseTensor2D.h"
#include "larcv/core/DataFormat/EventVoxel2D.h"

#include <random>

namespace larcv {

static BatchFillerSparseTensor2DProcessFactory
    __global_BatchFillerSparseTensor2DProcessFactory__;

BatchFillerSparseTensor2D::BatchFillerSparseTensor2D(const std::string name)
    : BatchFillerTemplate<float>(name) {}

void BatchFillerSparseTensor2D::configure(const PSet& cfg) {
  LARCV_DEBUG() << "start" << std::endl;
  _tensor2d_producer = cfg.get<std::string>("Tensor2DProducer");

  // Max voxels imposes a limit to make the memory layout regular.  Assuming average sparsity of x% , it's safe to
  // set _max_voxels to n_rows*n_cols*x*2 or so.  It's still a dramatic memory reduction.
  _max_voxels = cfg.get<float>("MaxVoxels", 0);
  _slice_v = cfg.get<std::vector<size_t> >("Channels", _slice_v);
  _include_values = cfg.get<bool>("IncludeValues", true);

  if (_max_voxels == 0){
    LARCV_CRITICAL() << "Maximum number of voxels must be non zero!" << std::endl;
    throw larbys();
  }

  _allow_empty = cfg.get<bool>("AllowEmpty",false);

  LARCV_DEBUG() << "done" << std::endl;
}

void BatchFillerSparseTensor2D::initialize() {}

void BatchFillerSparseTensor2D::_batch_begin_() {
  if(!batch_data().dim().empty() && (int)(batch_size()) != batch_data().dim().front()) {
    LARCV_INFO() << "Batch size changed " << batch_data().dim().front() << "=>" << batch_size() << std::endl;
    auto dim = batch_data().dim();
    dim[0] = batch_size();
    this->set_dim(dim);
  }
}

void BatchFillerSparseTensor2D::_batch_end_() {
  if (logger().level() <= msg::kINFO)
    LARCV_INFO() << "Total data size: " << batch_data().data_size()
                 << std::endl;
}

void BatchFillerSparseTensor2D::finalize() { _entry_data.clear(); }

void BatchFillerSparseTensor2D::assert_dimension(
    const EventSparseTensor2D& voxel_data) const {
  // auto const& voxel_meta = voxel_data.as_vector().front().meta();
  // if (_rows != voxel_meta.rows()) {
  //   LARCV_CRITICAL() << "# of Y-voxels (" << _rows << ") changed ... now "
  //                    << voxel_meta.rows() << std::endl;
  //   throw larbys();
  // }
  // if (_cols != voxel_meta.cols()) {
  //   LARCV_CRITICAL() << "# of X-voxels (" << _cols << ") changed ... now "
  //                    << voxel_meta.cols() << std::endl;
  //   throw larbys();
  // }
  return;
}

bool BatchFillerSparseTensor2D::process(IOManager& mgr) {
  
  /*

  Filling a dense tensor of dimensions B, H, W, C produces, in the end,
  a 4D tensor.  Here, we want to produce a list of points in sparse format.
  SO, this will be a 3 dimensional tensor of dimensions B, N_max, dims_len
  Where dims_len is the number of values that represent a point.
  In this 2D example, dims_len == 3 (H, W, Value) which are extracted from 
  the original image.

  In principle this can be bigger or smaller for different sized inputs:
  dims_len = 4 (H, W, D, Value) in 3D, for example

  The N_max value is a limit on how much memory is allocated for each event.
  The empty values are all set to 0.0, which are easily ignored in the 
  graph networks since the pooling layers are typically max reductions

  By convention, the Value component is always last in the network

  If there are more points in the input than are available in the output,
  The points are truncated.

  It's possible to do a random downsampling, but in case of segementation
  networks this needs to be coordinated across the image and label filler.

  */


  LARCV_DEBUG() << "start" << std::endl;
  auto const& voxel_data =
      mgr.get_data<larcv::EventSparseTensor2D>(_tensor2d_producer);
  if (!_allow_empty && voxel_data.as_vector().empty()) {
    LARCV_CRITICAL()
        << "Could not locate non-empty voxel data w/ producer name "
        << _tensor2d_producer << std::endl;
    throw larbys();
  }

  _num_channels = _slice_v.size();


  // if (_num_channels != voxel_data.as_vector().size()){
  //   LARCV_CRITICAL() << "Number of requested channels does not match number of available channels." << std::endl;
  //   throw larbys();
  // }

  int point_dim = 3; // This is hardcoded to be row, col, value, but could be extended
  if (!_include_values){
    point_dim = 2;
  }
  // one time operation: get image dimension
  if (batch_data().dim().empty()) {
    std::vector<int> dim;
    dim.resize(4);
    dim.at(0) = batch_size();
    dim.at(1) = _num_channels;
    dim.at(2) = _max_voxels;
    dim.at(3) = point_dim;
    this->set_dim(dim);
  } else
    this->assert_dimension(voxel_data);



  if (_entry_data.size() != batch_data().entry_data_size())
    _entry_data.resize(batch_data().entry_data_size(), 0.);


  // Reset all values to 0.0
  for (auto& v : _entry_data) v = 0.0;


  for ( auto const& voxel_set : voxel_data.as_vector()){
    auto & meta = voxel_set.meta();
    auto projection_id = meta.id();

    // Check that this projection ID is in the lists of channels:
    bool found = false;
    int count = 0;
    for (auto & channel : _slice_v){
      if (channel == projection_id){
        found = true;
        break;
      }
      count ++;
    }
    if (!found) continue;
    size_t i = 0;
    for (auto const& voxel : voxel_set.as_vector()) {
      int row = meta.index_to_row(voxel.id());
      int col = meta.index_to_col(voxel.id());
      size_t index = count*(_max_voxels*point_dim) + i*point_dim;
      _entry_data.at(count*(_max_voxels*point_dim) + i*point_dim) = row;
      _entry_data.at(count*(_max_voxels*point_dim) + i*point_dim + 1) = col;
      if (_include_values){
        _entry_data.at(count*(_max_voxels*point_dim) + i*point_dim + 2) = voxel.value();
      }
      i++;

      if (i == _max_voxels) {
        LARCV_INFO() << "Truncating the number of voxels!" << std::endl;
        break;
      }
    }

  }

  // record the entry data
  LARCV_INFO() << "Inserting entry data of size " << _entry_data.size()
               << std::endl;
  set_entry_data(_entry_data);

  return true;
}
}
#endif
