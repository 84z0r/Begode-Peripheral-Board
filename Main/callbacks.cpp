#include "callbacks.h"
#include "begode.h"

Begode::Hardware& Hardware = Begode::Hardware::Get();

void Setup()
{
    Hardware.onSetup();
}

void Loop()
{
    Hardware.onLoop();
}

//No callbacks for now...