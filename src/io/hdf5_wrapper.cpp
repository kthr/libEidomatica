/*
 * Copyright 2010-2011 Barry Wardell and Ian Hinder
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include "hdf5_wrapper.hpp"

H5Exception::H5Exception(const std::string& message)
  : message(message) {}

const char* H5Exception::getCMessage()
{
   return message.c_str();
}

/* Base wrapper class */
hid_t H5Base::getId() const
{
  return id;
}

/* H5F wrapper */
H5F::H5F(const std::string& filename)
{
  id = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  if( id<0 )
  {
    throw H5Exception("H5Fopen failed");
  }
}

H5F::~H5F()
{
  if( H5Fclose(id) < 0 )
  {
    throw H5Exception("H5Fclose failed");
  }
}

/* H5D wrapper */
H5D::H5D(const H5F& file, const std::string& dataset)
{
  id = H5Dopen(file.getId(), dataset.c_str(), H5P_DEFAULT);
  if( id<0 )
  {
    throw H5Exception("H5Dopen failed");
  }
}

H5D::~H5D()
{
  if( H5Dclose(id) < 0 )
    throw H5Exception("H5Dclose failed");
}

int H5D::getNumAttrs() const
{
  H5O_info_t object_info;
  H5Oget_info(id, &object_info);
  return object_info.num_attrs;
}

/* H5G wrapper */
H5G::H5G(const H5F& file, const std::string& dataset)
{
  id = H5Gopen(file.getId(), dataset.c_str(), H5P_DEFAULT);
  if( id<0 )
  {
    throw H5Exception("H5Dopen failed");
  }
}

H5G::~H5G()
{
  if( H5Gclose(id) < 0 )
    throw H5Exception("H5Dclose failed");
}

int H5G::getNumAttrs() const
{
  H5O_info_t object_info;
  H5Oget_info(id, &object_info);
  return object_info.num_attrs;
}

/* H5S wrapper */
H5S::H5S(const H5A& attr)
{
  id = H5Aget_space(attr.getId());
  if( id<0 )
    throw H5Exception("H5Aget_space failed");
}

H5S::H5S(const H5D& ds)
{
  id = H5Dget_space(ds.getId());
  if( id<0 )
    throw H5Exception("H5Dget_space failed");
}

H5S::~H5S()
{
  if( H5Sclose(id) < 0 )
    throw H5Exception("H5Sclose failed");
}

int H5S::getSimpleExtentDims(hsize_t *dims) const
{
  int ndims = H5Sget_simple_extent_dims(id, dims, NULL);
  if( ndims < 0 )
    throw H5Exception("H5Sget_simple_extent_dims failed");
  return ndims ;
}

int H5S::getSimpleExtentNDims() const
{
  int ndims = H5Sget_simple_extent_ndims(id);
  if( ndims < 0 )
    throw H5Exception("H5Sget_simple_extent_ndims failed");
  return ndims ;
}

/* H5T wrapper */
H5T::H5T(const H5D& ds)
{
  id = H5Dget_type(ds.getId());
  if( id<0 )
    throw H5Exception("H5Dget_type failed");

  native_id = H5Tget_native_type(id, H5T_DIR_ASCEND);
  if( native_id<0 )
    throw H5Exception("H5Tget_native_type failed");

  if (H5Tget_class(id) == H5T_ARRAY)
    super_id = H5Tget_super(id);
  else
    super_id = H5T_NO_CLASS;
}

H5T::H5T(const H5A& attr)
{
  id = H5Aget_type(attr.getId());
  if( id<0 )
    throw H5Exception("H5Aget_type failed");

  native_id = H5Tget_native_type(id, H5T_DIR_ASCEND);
  if( native_id<0 )
    throw H5Exception("H5Tget_native_type failed");
}

H5T::~H5T()
{
  if( H5Tclose(id)<0 )
    throw H5Exception("H5Tclose failed");
  if( H5Tclose(native_id)<0 )
    throw H5Exception("H5Tclose failed");
}

size_t H5T::getSize() const
{
  return H5Tget_size(id);
}

size_t H5T::getSuperSize() const
{
  if(super_id == H5T_NO_CLASS)
    return -1;
  else
    return H5Tget_size(super_id);
}

hid_t H5T::getNativeId() const
{
  return native_id;
}

hid_t H5T::getSuperId() const
{
  return super_id;
}

/* H5A wrapper */
H5A::H5A(hid_t location_id, const char* attr_name)
{
  id = H5Aopen(location_id, attr_name, H5P_DEFAULT);
  if( id<0 )
    throw H5Exception("H5Aopen failed");
}

H5A::~H5A()
{
  if( H5Aclose(id)<0 )
    throw H5Exception("H5Aclose failed");
}

H5O::H5O(const H5F& file, const std::string& dataset)
{
  id = H5Oopen(file.getId(), dataset.c_str(), H5P_DEFAULT);
  if( id<0 )
  {
    throw H5Exception("H5Oopen failed");
  }
}

H5O::~H5O()
{
  if( H5Oclose(id) < 0 )
    throw H5Exception("H5Oclose failed");
}

