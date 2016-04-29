#pragma once

class ISimulationEntity
{
public:
    ISimulationEntity() = default;
    virtual ~ISimulationEntity() = default;

    virtual void Simulate(double days) = 0;
};