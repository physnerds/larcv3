/**
 * \file EventVoxel2D.h
 *
 * \ingroup core_DataFormat
 *
 * \brief Class def header for a class larcv::EventSparseTensor and larcv::EventClusterPixel2D
 *
 * @author kazuhiro
 */

/** \addtogroup core_DataFormat

    @{*/
#ifndef __LARCV_EVENTVOXEL2D_H
#define __LARCV_EVENTVOXEL2D_H

#include <iostream>
#include "EventBase.h"
#include "DataProductFactory.h"
#include "Voxel.h"
#include "ImageMeta.h"


namespace larcv {

  /**
    \class EventClusterPixel2D
    \brief Event-wise class to store a collection of VoxelSet (cluster) per projection id
  */
  // class EventClusterPixel2D : public EventBase {

  // public:

  //   /// Default constructor
  //   EventClusterPixel2D() {}

  //   /// Default destructor
  //   virtual ~EventClusterPixel2D() {}

  //   /// EventBase::clear() override
  //   inline void clear() {EventBase::clear(); _cluster_v.clear();}

  //   /// Access to all stores larcv::ClusterPixel2D
  //   inline const std::vector<larcv::ClusterPixel2D>& as_vector() const { return _cluster_v; }

  //   /// Access ClusterPixel2D of a specific projection ID
  //   const larcv::ClusterPixel2D& cluster_pixel_2d(const ProjectionID_t id) const;

  //   /// Number of valid projection id
  //   inline size_t size() const { return _cluster_v.size(); }

  //   //
  //   // Write-access
  //   //
  //   /// Emplace data
  //   void emplace(larcv::ClusterPixel2D&& clusters);
  //   /// Set data
  //   void set(const larcv::ClusterPixel2D& clusters);
  //   /// Emplace a new element
  //   void emplace(larcv::VoxelSetArray&& clusters, larcv::ImageMeta&& meta);
  //   /// Set a new element
  //   void set(const larcv::VoxelSetArray& clusters, const larcv::ImageMeta& meta);

  // private:
  //   std::vector<larcv::ClusterPixel2D> _cluster_v;
    
  // };

  /**
    \class EventSparseTensor
    \brief Event-wise class to store a collection of VoxelSet (cluster) per projection id
  */
  class EventSparseTensor : public EventBase {

  public:

    /// Default constructor
    EventSparseTensor() {_is_initialized=false;}


    //
    // Read-access
    //
    /// Access to all stores larcv::SparseTensor
    inline const std::vector<larcv::SparseTensor>& as_vector() const { return _tensor_v; }
    /// Access SparseTensor of a specific projection ID
    const larcv::SparseTensor& sparse_tensor(const ProjectionID_t id) const;
    /// Number of valid projection id
    inline size_t size() const { return _tensor_v.size(); }

    //
    // Write-access
    //
    /// EventBase::clear() override
    void clear() {_tensor_v.clear();}
    /// Emplace data
    void emplace(larcv::SparseTensor&& voxels);
    /// Set data`
    void set(const larcv::SparseTensor& voxels);
    /// Emplace a new element
    void emplace(larcv::VoxelSet&& voxels, larcv::ImageMeta&& meta);
    /// Set a new element
    void set(const larcv::VoxelSet& voxels, const larcv::ImageMeta& meta);


    // IO functions:
    void initialize (H5::Group * group);
    void serialize  (H5::Group * group);
    void deserialize(H5::Group * group, size_t entry);


  private:
    void _initialize( H5::Group * group);
    bool _is_initialized;

    std::vector<larcv::SparseTensor>  _tensor_v;
    
  };

}

#include "IOManager.h"
namespace larcv {

  // // Template instantiation for IO
  // template<> inline std::string product_unique_name<larcv::EventClusterPixel2D>() { return "cluster2d"; }
  // template EventClusterPixel2D& IOManager::get_data<larcv::EventClusterPixel2D>(const std::string&);
  // template EventClusterPixel2D& IOManager::get_data<larcv::EventClusterPixel2D>(const ProducerID_t);

  // *
  //    \class larcv::EventClusterPixel2D
  //    \brief A concrete factory class for larcv::EventClusterPixel2D
  
  // class EventClusterPixel2DFactory : public DataProductFactoryBase {
  // public:
  //   /// ctor
  //   EventClusterPixel2DFactory()
  //   { DataProductFactory::get().add_factory(product_unique_name<larcv::EventClusterPixel2D>(), this); }
  //   /// dtor
  //   ~EventClusterPixel2DFactory() {}
  //   /// create method
  //   EventBase* create() { return new EventClusterPixel2D; }
  // };

  // Template instantiation for IO
  template<> inline std::string product_unique_name<larcv::EventSparseTensor>() { return "sparse"; }
  // template EventSparseTensor& IOManager::get_data<larcv::EventSparseTensor>(const std::string&);
  // template EventSparseTensor& IOManager::get_data<larcv::EventSparseTensor>(const ProducerID_t);

  /**
     \class larcv::EventSparseTensor
     \brief A concrete factory class for larcv::EventSparseTensor
  */
  class EventSparseTensorFactory : public DataProductFactoryBase {
  public:
    /// ctor
    EventSparseTensorFactory()
    { DataProductFactory::get().add_factory(product_unique_name<larcv::EventSparseTensor>(), this); }
    /// dtor
    ~EventSparseTensorFactory() {}
    /// create method
    EventBase* create() { return new EventSparseTensor; }
  };


}

#endif
/** @} */ // end of doxygen group
