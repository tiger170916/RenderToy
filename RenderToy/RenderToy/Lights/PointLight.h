#include "../Includes.h"
#include "LightExtension.h"
#include "../Vectors.h"

class PointLight : public LightExtension
{
private:
	FVector3 m_localPosition;

	FVector3 m_radiance;

public:
	PointLight(FVector3 localPosition, FVector3 radiance);

	~PointLight();
};