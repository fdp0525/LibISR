#pragma once
#include "../../Utils/LibISRDefine.h"
#include "../../../ORUtils/CUDADefines.h"

namespace LibISR
{
	namespace Objects
	{
		/**
		\brief
		object SDF
		this version don't contain any pose
		radiant of SDF are computed on the fly,
		so no gradient volume is kept

		refactored: Jan/13/2015
		*/
		class ISRShape
		{
		private:

			float *dt;
			float *dt_device;

		public:

			int objectId;
			Vector3i volSize;
			int allocatedSize;

			bool useGPU;
			bool modelShared;
			bool modelLoaded;

			_CPU_AND_GPU_CODE_ float* getSDFVoxel(bool fromGPU=false)
			{ 
				if (fromGPU) return dt_device;
				else return dt; 
			}
			_CPU_AND_GPU_CODE_ const float* getSDFVoxel(bool fromGPU = false) const
			{
				if (fromGPU) return dt_device;
				else return dt;
			}

			void  loadShapeFromFile(const char* fileName, Vector3i size = Vector3i(DT_VOL_SIZE))
			{
				volSize = size;
				allocatedSize = size.x*size.y*size.z;

				float *dt_host = new float[allocatedSize];

				FILE* f;
				f = fopen(fileName, "rb");
				fread(dt_host, sizeof(float) * this->allocatedSize, 1, f);
				fclose(f);

				if (useGPU)
				{
					ORcudaSafeCall(cudaMalloc((void**)&dt_device, allocatedSize*sizeof(float)));
					ORcudaSafeCall(cudaMemcpy(dt_device, dt_host, allocatedSize*sizeof(float), cudaMemcpyHostToDevice));
					//free(dt_host);
				}

				dt = dt_host;
				//else
				//{
				//	dt = dt_host;
				//}

				modelLoaded = true;
				modelShared = false;
			}

			void  loadShapeFromExistingShape(const ISRShape &shape)
			{
				volSize = shape.volSize;
				allocatedSize = shape.allocatedSize;
				useGPU = shape.useGPU;

				if (useGPU)
				{
					ORcudaSafeCall(cudaMalloc((void**)&dt_device, allocatedSize*sizeof(float)));
					ORcudaSafeCall(cudaMemcpy(dt_device, shape.getSDFVoxel(true), allocatedSize*sizeof(float), cudaMemcpyHostToDevice));
				}
				//else
				{
					dt = (float*)malloc(sizeof(float) * allocatedSize);
					memcpy(dt, shape.getSDFVoxel(), allocatedSize*sizeof(float));
				}

				modelLoaded = true;
				modelShared = false;
			}

			void  shareSDFWithExistingShape(ISRShape &shape)
			{
				volSize = shape.volSize;
				allocatedSize = shape.allocatedSize;
				useGPU = shape.useGPU;

				dt = shape.getSDFVoxel();
				dt_device = shape.getSDFVoxel(true);

				modelLoaded = true;
				modelShared = true;
			}

			void initialize(bool usegpu, int id)
			{
				objectId = id;
				useGPU = usegpu;

				modelLoaded = false;
				modelShared = false;
			}

			ISRShape(){}
			~ISRShape()
			{
				if (modelLoaded && !modelShared && dt != NULL)
				{
					if (useGPU) ORcudaSafeCall(cudaFree(dt)); 
					//else 
						free(dt);
				}
			}
		};

		typedef ISRShape* ISRShape_ptr;
	}
}