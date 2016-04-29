#pragma once

#include <memory>

#include "isimulationentity.h"

class Bank;
class Account : public ISimulationEntity
{
private:
    Bank& bank_;
    double balance_;
    double annual_rate_;
public:
    Account(Bank& bank, double balance, double annual_rate);
    
    double GetBalance() const;
    
    void Deposit(double value);
    
    double Withdraw(double value);
    
    virtual void Simulate(double days);
    
};

typedef std::shared_ptr<Account> AccountPtr;