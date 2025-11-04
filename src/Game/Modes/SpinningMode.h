#pragma once
#include "Layer/Layer.h"

class SpinningMode : public Layer
{
public:
	~SpinningMode();
	void update(float dt) override;
};
