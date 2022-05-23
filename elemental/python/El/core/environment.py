#
#  Copyright (c) 2009-2016, Jack Poulson
#  All rights reserved.
#
#  This file is part of Elemental and is under the BSD 2-Clause License, 
#  which can be found in the LICENSE file in the root directory, or at 
#  http://opensource.org/licenses/BSD-2-Clause
#
import sys, ctypes
import numpy as np
from ctypes import pythonapi

lib = ctypes.cdll.LoadLibrary("/home/alchemist/install/elemental/lib64/libEl.so")

# Environment
# ===========

# Basic types
# -----------

from ctypes import c_bool, c_size_t, c_ubyte, c_uint, c_int, c_float, c_double
from ctypes import c_longlong, c_void_p, c_char_p
from ctypes import pointer, POINTER

bType = c_bool

# Determine whether we have 64-bit integers or not
lib.ElUsing64BitInt.argtypes = [POINTER(bType)]
using64 = bType()
lib.ElUsing64BitInt(pointer(using64))
if using64:
  iType = c_longlong
  iSize = 8
  iNpType = np.int64
else:
  iType = c_int
  iSize = 4
  iNpType = np.int32

sType = c_float
sSize = 4
sNpType = np.float32

dType = c_double
dSize = 8
dNpType = np.float64

class ComplexFloat(ctypes.Structure):
  _fields_ = [("real",sType),("imag",sType)]
  def __init__(self,val0=0,val1=0):
    real = sType()
    imag = sType()
    if type(val0) is cType or type(val0) is zType:
      real = val0.real
      imag = val0.imag
    else:
      real = val0
      imag = val1
    super(ComplexFloat,self).__init__(real,imag)
cType = ComplexFloat
cSize = 2*sSize
cNpType = np.complex64

class ComplexDouble(ctypes.Structure):
  _fields_ = [("real",dType),("imag",dType)]
  def __init__(self,val0=0,val1=0):
    real = dType()
    imag = dType()
    if type(val0) is cType or type(val0) is zType:
      real = val0.real
      imag = val0.imag
    else:
      real = val0
      imag = val1
    super(ComplexDouble,self).__init__(real,imag)
zType = ComplexDouble
zSize = 2*dSize
zNpType = np.complex128

# Create a simple enum for the supported datatypes
(iTag,sTag,dTag,cTag,zTag)=(0,1,2,3,4)
def CheckTag(tag):
  if tag != iTag and \
     tag != sTag and tag != dTag and \
     tag != cTag and tag != zTag:
    print 'Unsupported datatype'

def Base(tag):
  if   tag == iTag: return iTag
  elif tag == sTag: return sTag
  elif tag == dTag: return dTag
  elif tag == cTag: return sTag
  elif tag == zTag: return dTag
  else: raise Exception('Invalid tag')

def Complexify(tag):
  if   tag == sTag: return cTag
  elif tag == dTag: return zTag
  elif tag == cTag: return cTag
  elif tag == zTag: return zTag
  else: raise Exception('Invalid tag')

# While for CTypes we should return the underlying value of, e.g., c_int,
# we cannot/should-not do the same for ComplexDouble
def ScalarData(alpha):
  if   type(alpha) is iType: return alpha.value
  elif type(alpha) is sType: return alpha.value
  elif type(alpha) is dType: return alpha.value
  else: return alpha

def TagToType(tag):
  if   tag == iTag: return iType
  elif tag == sTag: return sType
  elif tag == dTag: return dType
  elif tag == cTag: return cType
  elif tag == zTag: return zType
  else: raise Exception('Invalid tag')

def TagToSize(tag):
  if   tag == iTag: return iSize
  elif tag == sTag: return sSize
  elif tag == dTag: return dSize
  elif tag == cTag: return cSize
  elif tag == zTag: return zSize
  else: raise Exception('Invalid tag')

def TagToNumpyType(tag):
  if   tag == iTag: return iNpType
  elif tag == sTag: return sNpType
  elif tag == dTag: return dNpType
  elif tag == cTag: return cNpType
  elif tag == zTag: return zNpType
  else: raise Exception('Invalid tag')

# Emulate an enum for matrix distributions
(MC,MD,MR,VC,VR,STAR,CIRC)=(0,1,2,3,4,5,6)

def GatheredDist(dist):
  if   dist == CIRC: return CIRC
  else: return STAR

def DiagColDist(colDist,rowDist):
  if colDist == MC and rowDist == MR:   return MD
  elif colDist == MR and rowDist == MC: return MD
  elif colDist == STAR:                 return rowDist
  else:                                 return colDist

def DiagRowDist(colDist,rowDist):
  if colDist == MC and rowDist == MR:   return STAR
  elif colDist == MR and rowDist == MC: return STAR
  elif colDist == STAR:                 return colDist
  else:                                 return rowDist

class IndexRange(ctypes.Structure):
  _fields_ = [("beg",iType),("end",iType)]
  def __init__(self,ind0,ind1=None):
    beg = iType()
    end = iType()
    if isinstance(ind0,slice):
      if ind0.step != None and ind0.step != 1:
        raise Exception('Slice step must be one')
      beg = iType(ind0.start)
      end = iType(ind0.stop)
    else:
      beg = iType(ind0)
      if ind1 is None: end = iType(beg.value+1)
      else:            end = iType(ind1)
    super(IndexRange,self).__init__(beg,end)

# Emulate various enums
# ---------------------

# Grid ordering
(ROW_MAJOR,COL_MAJOR)=(0,1)

# Upper or lower
(LOWER,UPPER)=(0,1)

# Left or right
(LEFT,RIGHT)=(0,1)

# Matrix orientation 
(NORMAL,TRANSPOSE,ADJOINT)=(0,1,2)

# Unit/non-unit diagonal
(NON_UNIT,UNIT)=(0,1)

# File format 
(AUTO,ASCII,ASCII_MATLAB,BINARY,BINARY_FLAT,BMP,JPG,JPEG,MATRIX_MARKET,
 PNG,PPM,XBM,XPM)=(0,1,2,3,4,5,6,7,8,9,10,11,12)

# Colormap
(GRAYSCALE,GRAYSCALE_DISCRETE,RED_BLACK_GREEN,BLUE_RED)=(0,1,2,3)

# Norm types
(ONE_NORM,INFINITY_NORM,ENTRYWISE_ONE_NORM,MAX_NORM,NUCLEAR_NORM,FROBENIUS_NORM,
 TWO_NORM)=(0,1,2,3,4,5,6)

# Sort type
(UNSORTED,DESCENDING,ASCENDING)=(0,1,2)

# Pencil
(AXBX,ABX,BAX)=(1,2,3)

# TODO: Many more enums

# Miscellaneous small data structures
# -----------------------------------
class SafeProduct_s(ctypes.Structure):
  _fields_ = [("rho",sType),("kappa",sType),("n",iType)]
class SafeProduct_d(ctypes.Structure):
  _fields_ = [("rho",dType),("kappa",dType),("n",iType)]
class SafeProduct_c(ctypes.Structure):
  _fields_ = [("rho",cType),("kappa",sType),("n",iType)]
class SafeProduct_z(ctypes.Structure):
  _fields_ = [("rho",zType),("kappa",dType),("n",iType)]
def TagToSafeProduct(tag):
  if   tag == sTag: return SafeProduct_s()
  elif tag == dTag: return SafeProduct_d()
  elif tag == cTag: return SafeProduct_c()
  elif tag == zTag: return SafeProduct_z()
  else: DataExcept()

class InertiaType(ctypes.Structure):
  _fields_ = [("numPositive",iType),("numNegative",iType),("numZero",iType)]

# Initialization
# --------------

lib.ElInitialize.argtypes = [POINTER(c_int),POINTER(POINTER(c_char_p))]
def Initialize():
  argc = c_int()
  argv = POINTER(c_char_p)()
  pythonapi.Py_GetArgcArgv(ctypes.byref(argc),ctypes.byref(argv))
  lib.ElInitialize(pointer(argc),pointer(argv))

lib.ElInitialized.argtypes = [POINTER(bType)]
def Initialized():
  active = bType()
  lib.ElInitialized( pointer(active) )
  return active.value

lib.ElSetBlocksize.argtypes = [iType]
def SetBlocksize(blocksize):
  lib.ElSetBlocksize(blocksize)

lib.ElBlocksize.argtypes = [POINTER(iType)]
def Blocksize():
  blocksize = iType()
  lib.ElBlocksize(pointer(blocksize))
  return blocksize

# Import several I/O routines
# ---------------------------
pythonapi.PyFile_AsFile.argtypes = [ctypes.py_object]
pythonapi.PyFile_AsFile.restype = c_void_p

lib.ElPrintVersion.argtypes = [c_void_p]
def PrintVersion(f=pythonapi.PyFile_AsFile(sys.stdout)):
  lib.ElPrintVersion(f)

lib.ElPrintConfig.argtypes = [c_void_p]
def PrintConfig(f=pythonapi.PyFile_AsFile(sys.stdout)):
  lib.ElPrintConfig(f)

lib.ElPrintCCompilerInfo.argtypes = [c_void_p]
def PrintCCompilerInfo(f=pythonapi.PyFile_AsFile(sys.stdout)):
  lib.ElPrintCCompilerInfo(f)

lib.ElPrintCxxCompilerInfo.argtypes = [c_void_p]
def PrintCxxCompilerInfo(f=pythonapi.PyFile_AsFile(sys.stdout)):
  lib.ElPrintCxxCompilerInfo(f)

# Input routines
# --------------
# TODO?

# Initialize MPI
# --------------
Initialize()

def TypeExcept():
  raise Exception('Unsupported matrix type')
def DataExcept():
  raise Exception('Unsupported datatype')
