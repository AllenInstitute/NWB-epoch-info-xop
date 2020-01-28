#include "mies-nwb2-compound-XOP_handler.h"

#include "CustomExceptions.h"
#include "H5Cpp.h"
#include "H5Exception.h"
#include "Helpers.h"
#include "Operations.h"
#include "xop_errors.h"
#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace
{
using namespace fmt::literals;

static const std::string MEMBERNAME_START = "idx_start";
static const std::string MEMBERNAME_COUNT = "count";
static const std::string MEMBERNAME_REF   = "timeseries";
static const int MEMBERNUMBER             = 3;
static const int MEMBERNAME_START_IDX     = 0;
static const int MEMBERNAME_COUNT_IDX     = 1;
static const int MEMBERNAME_REF_IDX       = 2;

typedef struct dataPoint
{
  int offset;
  int size;
  hobj_ref_t ref;
} dataPoint;

void CheckCompoundMemberType(const H5::CompType &compType, const int index, const H5::PredType &predType)
{
  if(!(compType.getMemberDataType(index) == predType))
  {
    throw IgorException(ERR_INVALID_TYPE, "Referenced HDF5 compound member has wrong type.");
  }
}

} // namespace

Handler &XOPHandler()
{
  return Handler::Instance();
}

Handler &Handler::Instance()
{
  static Handler h;

  return h;
}

void Handler::IPNWB_WriteCompound(IPNWB_WriteCompoundRuntimeParamsPtr p)
{

  if(!p->SFlagEncountered || !p->CFlagEncountered || !p->REFFlagEncountered || !p->LOCFlagEncountered ||
     !p->fullFileNameEncountered)
  {
    throw IgorException(ERR_FLAGPARAMS, "Parameter(s) missing.");
  }

  auto fileName = GetStringFromHandle(p->fullFileName);
  if(fileName.empty())
  {
    throw IgorException(ERR_INVALID_TYPE, "File name missing.");
  }
  auto compPath = GetStringFromHandle(p->compPath);
  if(compPath.empty())
  {
    throw IgorException(ERR_INVALID_TYPE, "HDF5 data path missing.");
  }

  if(p->tsRefWave == nullptr)
  {
    throw IgorException(ERR_INVALID_TYPE, "Reference wave is null.");
  }
  if(WaveType(p->tsRefWave) != TEXT_WAVE_TYPE)
  {
    throw IgorException(ERR_INVALID_TYPE, "Reference wave has wrong type.");
  }
  auto tsRefWaveDims = GetWaveDimension(p->tsRefWave);
  if(tsRefWaveDims[1] > 0)
  {
    throw IgorException(ERR_INVALID_TYPE, "Reference wave must be 1D.");
  }
  if(p->sizeWave == nullptr)
  {
    throw IgorException(ERR_INVALID_TYPE, "Size wave is null.");
  }
  if(WaveType(p->sizeWave) != NT_I32)
  {
    throw IgorException(ERR_INVALID_TYPE, "Size wave has wrong type.");
  }
  auto sizeWaveDims = GetWaveDimension(p->sizeWave);
  if(sizeWaveDims[1] > 0)
  {
    throw IgorException(ERR_INVALID_TYPE, "Size wave must be 1D.");
  }
  if(p->offsetWave == nullptr)
  {
    throw IgorException(ERR_INVALID_TYPE, "Offset wave is null.");
  }
  if(WaveType(p->offsetWave) != NT_I32)
  {
    throw IgorException(ERR_INVALID_TYPE, "Offset wave has wrong type.");
  }
  auto offsetWaveDims = GetWaveDimension(p->offsetWave);
  if(offsetWaveDims[1] > 0)
  {
    throw IgorException(ERR_INVALID_TYPE, "Offset wave must be 1D.");
  }
  if(!(sizeWaveDims[0] == offsetWaveDims[0] && sizeWaveDims[0] == tsRefWaveDims[0]))
  {
    throw IgorException(ERR_INVALID_TYPE, "Waves must have the same size");
  }

  try
  {
    hsize_t dims = sizeWaveDims[0];
    H5::CompType compType(sizeof(dataPoint));
    compType.insertMember(MEMBERNAME_START, 0, H5::PredType::STD_I32LE);
    compType.insertMember(MEMBERNAME_COUNT, 4, H5::PredType::STD_I32LE);
    compType.insertMember(MEMBERNAME_REF, 8, H5::PredType::STD_REF_OBJ);

    H5::H5File file(fileName, H5F_ACC_RDWR);

    auto compoundData = std::vector<dataPoint>(sizeWaveDims[0]);

    std::vector<IndexInt> dimCnt(MAX_DIMENSIONS, 0);
    for(auto &dp : compoundData)
    {
      dp.offset = GetWaveElement<int>(p->offsetWave, dimCnt);
      dp.size   = GetWaveElement<int>(p->sizeWave, dimCnt);
      file.reference(&dp.ref, GetWaveElement<std::string>(p->tsRefWave, dimCnt));
      dimCnt[0]++;
    }

    if(file.exists(compPath))
    {
      H5::DataSet dataSet = file.openDataSet(compPath);
      if(dataSet.getCreatePlist().getLayout() != H5D_CHUNKED)
      {
        throw IgorException(ERR_HDF5, "Existing dataset is not "
                                      "chunked. Can not append new data.");
      }

      hsize_t oldSize = dataSet.getSpace().getSelectNpoints();
      hsize_t newSize = oldSize + dims;
      dataSet.extend(&newSize);

      H5::DataSpace extFileDataSpace = dataSet.getSpace();
      extFileDataSpace.selectHyperslab(H5S_SELECT_SET, &dims, &oldSize);
      H5::DataSpace memDataSpace(1, &dims, nullptr);

      dataSet.write(compoundData.data(), compType, memDataSpace, extFileDataSpace);
    }
    else
    {
      hsize_t maxDims = H5S_UNLIMITED;
      H5::DataSpace dataSpace(1, &dims, &maxDims);

      H5::DSetCreatPropList dsetPropList;
      hsize_t chunkSize = 1;
      // note: layout is set to H5D_CHUNKED automatically.
      dsetPropList.setChunk(1, &chunkSize);
      H5::DataSet dataSet = file.createDataSet(compPath, compType, dataSpace, dsetPropList);

      dataSet.write(compoundData.data(), compType);
    }

    file.close();
  }
  catch(H5::Exception ex)
  {
    throw IgorException(ERR_HDF5, ex.getCDetailMsg());
  }
}

void Handler::IPNWB_ReadCompound(IPNWB_ReadCompoundRuntimeParamsPtr p)
{

  if(!p->SFlagEncountered || !p->CFlagEncountered || !p->REFFlagEncountered || !p->LOCFlagEncountered ||
     !p->fullFileNameEncountered)
  {
    throw IgorException(ERR_FLAGPARAMS, "Parameter(s) missing.");
  }
  auto fileName = GetStringFromHandle(p->fullFileName);
  if(fileName.empty())
  {
    throw IgorException(ERR_INVALID_TYPE, "File name missing.");
  }
  auto compPath = GetStringFromHandle(p->compPath);
  if(compPath.empty())
  {
    throw IgorException(ERR_INVALID_TYPE, "HDF5 data path missing.");
  }

  auto niceRefs      = std::vector<std::string>();
  auto offsets       = std::vector<int>();
  auto sizes         = std::vector<int>();
  hssize_t numPoints = 0;

  try
  {
    H5::H5File file(fileName, H5F_ACC_RDONLY);
    if(!file.exists(compPath))
    {
      file.close();
      throw IgorException(ERR_INVALID_TYPE, "HDF5 data not present at given path.");
    }
    H5::DataSet dataSet = file.openDataSet(compPath);

    H5::DataType dataType = dataSet.getDataType();
    H5::CompType compType(sizeof(dataPoint));
    if(!dataType.detectClass(compType.getClass()))
    {
      throw IgorException(ERR_INVALID_TYPE, "Referenced HDF5 dataset has not compound type.");
    }
    compType = H5::CompType(dataSet);
    if(compType.getNmembers() != MEMBERNUMBER)
    {
      throw IgorException(ERR_INVALID_TYPE, "Referenced HDF5 compound has not {} members."_format(MEMBERNUMBER));
    }
    int memIndexStart = compType.getMemberIndex(MEMBERNAME_START);
    CheckCompoundMemberType(compType, memIndexStart, H5::PredType::STD_I32LE);
    int memIndexCount = compType.getMemberIndex(MEMBERNAME_COUNT);
    CheckCompoundMemberType(compType, memIndexCount, H5::PredType::STD_I32LE);
    int memIndexRef = compType.getMemberIndex(MEMBERNAME_REF);
    CheckCompoundMemberType(compType, memIndexRef, H5::PredType::STD_REF_OBJ);
    if((memIndexStart != MEMBERNAME_START_IDX) || (memIndexCount != MEMBERNAME_COUNT_IDX) ||
       (memIndexRef != MEMBERNAME_REF_IDX))
    {
      throw IgorException(ERR_INVALID_TYPE, "Referenced HDF5 compound member has wrong element order.");
    }

    numPoints         = dataSet.getSpace().getSelectNpoints();
    auto compoundData = std::vector<dataPoint>(numPoints);
    dataSet.read(compoundData.data(), compType);

    for(const auto &dp : compoundData)
    {
      // dereferencing changes dset internally to a H5::Object, so we need a
      // fresh one to reapply dereference
      H5::DataSet dset = H5::DataSet();
      dset.dereference(file, &dp.ref);
      niceRefs.push_back(dset.getObjName());
      offsets.push_back(dp.offset);
      sizes.push_back(dp.size);
    }

    file.close();
  }
  catch(H5::Exception ex)
  {
    throw IgorException(ERR_HDF5, ex.getCDetailMsg());
  }

  auto dimCnt = std::vector<CountInt>(MAX_DIMENSIONS + 1, 0);
  dimCnt[0]   = numPoints;
  {
    auto checkWaveProperties = [](waveHndl w) {
      if(WaveType(w) != TEXT_WAVE_TYPE)
      {
        throw IgorException(ERR_INVALID_TYPE, "Only text waves are supported with /REF.");
      }
    };

    auto typeGetter = [](waveHndl /*unused*/) { return TEXT_WAVE_TYPE; };

    auto setWaveContents = [&](waveHndl w) { StringVectorToTextWave(niceRefs, w); };

    HandleDestWave(p->REFFlagParamsSet[0], p->tsRefWave, p->FREEFlagEncountered, dimCnt, checkWaveProperties,
                   typeGetter, setWaveContents);
  }
  {
    auto checkWaveProperties = [](waveHndl w) {
      if(WaveType(w) != NT_I32)
      {
        throw IgorException(ERR_INVALID_TYPE, "Only integer waves are supported with /S.");
      }
    };

    auto typeGetter = [](waveHndl /*unused*/) { return NT_I32; };

    auto setWaveContents = [&](waveHndl w) { std::memcpy(WaveData(w), offsets.data(), numPoints * sizeof(int)); };

    HandleDestWave(p->SFlagParamsSet[0], p->offsetWave, p->FREEFlagEncountered, dimCnt, checkWaveProperties, typeGetter,
                   setWaveContents);
  }
  {
    auto checkWaveProperties = [](waveHndl w) {
      if(WaveType(w) != NT_I32)
      {
        throw IgorException(ERR_INVALID_TYPE, "Only integer waves are supported with /C.");
      }
    };

    auto typeGetter = [](waveHndl /*unused*/) { return NT_I32; };

    auto setWaveContents = [&](waveHndl w) { std::memcpy(WaveData(w), sizes.data(), numPoints * sizeof(int)); };

    HandleDestWave(p->CFlagParamsSet[0], p->sizeWave, p->FREEFlagEncountered, dimCnt, checkWaveProperties, typeGetter,
                   setWaveContents);
  }
}

void Handler::SetQuietMode(bool quietMode)
{
  m_quietMode = quietMode;
}
