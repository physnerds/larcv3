## Dataformat Schema.

This document outlines the principles of data product design and serialization using hdf5.

In general, you can read this document to get an understanding of how the data products fit together, though you may or may not need to read it to use larcv effectively.

(As of 3/4/19, the dataformat package consists of very lightweight objects to test IO and serialization.)

# Core Data Products:
* EventBase (contains info about run/subrun/event, producer).  All dataproducts inherit from EventBase in their serialization wrapper.  The core data products do not, however the serialization wrapper (Typically named EventProduct, as in Particle is wrapped by EventParticle) will inherit from EventBase.
* Voxel (and derivative collections VoxelSet, VoxelSetArray).  
    * A voxel is effectively a two element object: an unsigned integer (typically size_t) for an index, and a floating point value.  Lone voxels are never serialized, though they can be created.  It is more efficient to add an (index, float) pair to a VoxelSet.
    * A VoxelSet is class containing two std::vectors of identical length.  There is an index vector and a value vector, which combined form a set of voxels.  A VoxelSet is contrained to hold voxels only from one projection if in 2D.  This is to enforce the notion that each voxel set gets it's own meta information.  Since projection ID is not a property of voxels, VoxelSet must hold an optional projection ID 
    * A VoxelSetArray contains a series of VoxelSets.  This can be a series of VoxelSets for semantic segmentation (or instance segmentation), or a series of multiple projections, or both.  VoxelSetArray stores pointers to VoxelSets to allow each voxel set to grow dynamically if needed, without having to reshuffle the other voxel sets.
    * VoxelSetArrays have functions to perform simple slicing on VoxelSets.  You can ask for all VoxelSets with a given projectionID, for example.
* ImageMeta (which covers 2D, 3D).  This class is to interpret a voxel index as a N-tuple coordinate.  It allows bulk conversion of voxel index to tuple'd indexes, suitable for numpy array indexes.
* Particle.  This class is the primary label based class.  It can be used flexibly to store event-level labels, or individual particle labels.  Since this repository is designed for high energy physics, the particle object is able to store properties like momentum, energy, etc.  It is stored as a flat table and can easily be inspected with hdfview.

Not all of the core data products are reimplemented for larcv3 yet.  In particular, only the following have been completed with the new IO model.

# File Organization

The serialization model depends on a carefully organized structure within the file.  To this end, there will be several main groups.

### Group: Events

This group will have one table, the events table, with the columns run/subrun/event.  This table will be used in every other dataproduct at load time so that there is a consistent, and provides the mapping from entry number (index in this table) to run/subrun/event.  The table will be called event_index

### Group: Data

This group contains the meat of the data in a file.  It will have a number of subgroups with the naming convention:
dataproduct_producername_group

Additionally, there may be a final subgroup of contents which holds the list of groups and their producers, and potentially their history.

### How is a data subgroup organized?

For any given subgroup, there will be at least two datasets inside of the group:
* event_extents
* data
* [extents_1]
* [data_2]
* [extents_2]
* ...

The data tables holds the actual content of the dataproduct.  For VoxelSets and their permutations, this will mean a table that has dimensions (N, 2), where N is the total number of voxels in a file.  For an image, the shape might be (N, 512, 512).

The data and it's access/organization is described by the extents tables.  So, the access to the table data_1 will be governed by extents_1.


### Data Product Interface with Serialization

All serialization is handled through the Event* classes (EventParticle, EventVoxel, etc).  In general, each of these classes inherits from EventBase, and contains a vector of their respective dataproducts.  Each class will have to manage flattening and unflattening of it's respective data.  So, for example, EventParticle inherits from EventBase and contains an std::vector<larcv::particle>.  For serialization, 
1. On READING, the IO process will look in the group data/particle_producername_group for the tables:
    * data
    * extents
2. From the event_extents table, which instructs the upper and lower index in the table, the list of particles is read in.
    * The EventParticle class will interpret the data as an array of particle objects, which can be wrapped in an std::vector and returned to the user.

1. On WRITING, the first step will be to update the EventBase Table with the event object for this file.
2. For the EventParticle class, the vector of particles is assumed to be static once the write function is called.
    1. The extents table is appended with the right numbers
    2. The data table is appended with the new data


# Serialization Overview

There are several steps in the workflow of data serialization.  I will first describe the WRITE mode serialization model, then the READ mode.  Finally, I will describe how they can be run simultaneously (that is, a COPY or APPEND operation)

## WRITE Mode Serialization

1. When IOManager's Initialize function is called, an output file is opened.
    * The output file is managed exclusively by IOManager
2. After the output file is opened, the top level groups are created ('Events', and 'Data')
3. The event ID table is created
4. Control passes to the processing loop (whatever is driving it, python or ProcessDriver or ThreadIO)
    * Data products are created in memory using the get_data functions.
    * When a data product is created for the first time, it's subgroup must be created under the 'Data' group.
    * Initialization of datasets inside of a subgroup is managed by the Event* class as serialization is object dependent. 
5. When one iteration of the processing event loop is finished and IOManager::save_entry() is called, all data products are assumed to be finalized.  At this point, the serialization of each object is passed to it's individual Event* class.
    * Each class serializes it's own data into it's tabular format, and clears it's internal data buffers.









