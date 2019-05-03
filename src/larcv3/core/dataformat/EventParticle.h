/**
 * \file EventParticle.h
 *
 * \ingroup DataFormat
 * 
 * \brief Class def header for a class EventParticle
 *
 * @author kazuhiro
 */

/** \addtogroup DataFormat

    @{*/
#ifndef __LARCV3DATAFORMAT_EVENTPARTICLE_H
#define __LARCV3DATAFORMAT_EVENTPARTICLE_H

#include "larcv3/core/dataformat/EventBase.h"
#include "larcv3/core/dataformat/Particle.h"
#include "larcv3/core/dataformat/DataProductFactory.h"

class EventParticle;

namespace larcv3 {
  /**
    \class EventParticle
    User-defined data product class (please comment!)
  */
  class EventParticle : public EventBase {
    
  public:
    
    /// Default constructor
    EventParticle(){}
    
    /// Default destructor
    ~EventParticle(){}


    

    void set(const std::vector<larcv3::Particle>& part_v);
    void append(const larcv3::Particle& part);
    void emplace_back(larcv3::Particle&& part);
    void emplace(std::vector<larcv3::Particle>&& part_v);

    inline const std::vector<larcv3::Particle>& as_vector() const
    { return _part_v; }

    inline size_t size() const {return _part_v.size();}

// #ifndef SWIG
    /// Data clear method
    void clear();
    void initialize (H5::Group * group);
    void serialize  (H5::Group * group);
    void deserialize(H5::Group * group, size_t entry);
// #endif

    static EventParticle * to_particle(EventBase * e){
      return (EventParticle *) e;
    }

  private:


    std::vector<larcv3::Particle> _part_v; ///< a collection of particles (index maintained)

  };
}


#include "IOManager.h"
namespace larcv3 {

  // Template instantiation for IO
  template<> 
  inline std::string product_unique_name<larcv3::EventParticle>() { return "particle"; }
  // template<> 
  // inline EventParticle& IOManager::get_data(const std::string&);
  // template<> 
  // inline EventParticle& IOManager::get_data(const ProducerID_t);

  /**
     \class larcv3::EventParticle
     \brief A concrete factory class for larcv3::EventParticle
  */
  class EventParticleFactory : public DataProductFactoryBase {
  public:
    /// ctor
    EventParticleFactory()
    { DataProductFactory::get().add_factory(product_unique_name<larcv3::EventParticle>(),this); }
    /// dtor
    ~EventParticleFactory() {}
    /// create method
    EventBase* create() { return new EventParticle; }
  };

  
}


#endif
/** @} */ // end of doxygen group 
