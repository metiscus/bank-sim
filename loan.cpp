#include "loan.h"
#include "bank.h"

#include <cmath>
#include <cstdio>


Loan::Loan(Bank& bank, double principal, double apr, uint32_t term)
    : annual_rate_(apr)
    , monthly_payment_(0)
    , term_remaining_(term)
    , bank_(bank)
{
    // request the principal amount from the bank
    principal = bank_.WithdrawCash(principal);
    
    // calculate the payments
    const double monthly_rate = (apr / 12.0);
    const double rate_factor = pow(1.0 + monthly_rate, term);
    monthly_payment_ = principal * (monthly_rate * rate_factor) / (rate_factor - 1.0);
}

double Loan::GetPaymentAmount() const
{
    return monthly_payment_;
}

uint64_t Loan::GetMonthsRemaining() const
{
    return term_remaining_ / 30;
}

void Loan::MakePayment()
{
    term_remaining_ -= 1;
    bank_.DepositCash(monthly_payment_);
}

bool Loan::IsPaidOff() const
{
    return term_remaining_ == 0;
}

void Loan::Simulate(double days)
{
    ;
}

double Loan::GetBalance() const
{
    return monthly_payment_ * term_remaining_;
}