#pragma once

#include "isimulationentity.h"

#include <cstdint>
#include <memory>

class Bank;
class Loan : public ISimulationEntity
{
private:
    double   annual_rate_;
    double   monthly_payment_;
    uint32_t term_remaining_;
    Bank&    bank_;

public:
    Loan(Bank& bank, double principal, double apr, uint32_t term);
    
    double       GetPaymentAmount() const;
    uint64_t     GetMonthsRemaining() const;
    bool         IsPaidOff() const;
    void         MakePayment();
    virtual void Simulate(double days);
    double       GetBalance() const;
};

typedef std::shared_ptr<Loan> LoanPtr;