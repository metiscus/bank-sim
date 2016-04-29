#include "bank.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

double Bank::reserve_requirement_ = 0.1;
double Bank::base_interest_rate_  = 0.045;

Bank::Bank(double cash_on_hand)
    : cash_on_hand_(cash_on_hand)
    , account_number_(1)
{
    
}

void Bank::SetReserveRequirement(double requirement)
{
    reserve_requirement_ = requirement;
}

void Bank::SetBaseInterestRate(double rate)
{
    base_interest_rate_ = rate;
}

double Bank::GetReserveRequirement()
{
    return reserve_requirement_;
}

double Bank::GetBaseInterestRate()
{
    return base_interest_rate_;
}

void Bank::Simulate(double days)
{
    for(auto& account : accounts_)
    {
        account.second->Simulate(days);
    }

    for(auto& loan : loans_)
    {
        loan.second->Simulate(days);
    }
    
    //assert(CheckWithdrawReserveRequirement(0));
}

double Bank::GetCashOnHand() const
{
    return cash_on_hand_;
}

void Bank::DepositCash(double cash)
{
    cash_on_hand_ += cash;
}

double Bank::WithdrawCash(double cash)
{
    double withdrawn = 0.0;

    if(CheckWithdrawReserveRequirement(cash))
    {
        cash_on_hand_ -= cash;
        withdrawn = cash;
    }
    return withdrawn;
}

uint64_t Bank::OpenAccount(double rate, double deposit)
{
    auto account = std::make_shared<Account>(*this, deposit, rate * 0.1);
    uint64_t account_number = account_number_++;
    accounts_.insert(std::make_pair(account_number, account));
    return account_number;
}

AccountPtr Bank::GetAccount(uint64_t number)
{
    assert(accounts_.count(number) == 1);
    return accounts_[number];
}

bool Bank::HasAccount(uint64_t number) const
{
    return accounts_.count(number) == 1;
}

uint64_t Bank::OpenLoan(double rate, double amount)
{
    // Ensure that the bank has the cash on hand to make the loan
    //TODO: eventually implement borrowing from central bank
    if(CheckWithdrawReserveRequirement(amount))
    {
        double rate_offset = 0.0006 * (1.0 / sqrt(0.001 + ComputeAssetRatio())/ 0.1);
        //printf("asset ratio: %lf rate_offset: %lf\n", ComputeAssetRatio(), rate_offset);
        uint64_t term = 6 + rand() % 30;
        
        auto loan = std::make_shared<Loan>(*this, amount, rate + rate_offset, term);
        uint64_t account_number = account_number_++;
        loans_.insert(std::make_pair(account_number, loan));
        return account_number;
    }

    return 0;
}

double Bank::ComputeAssetRatio() const
{
    double liabilities = 0.0;
    for(auto account : accounts_)
    {
        liabilities += account.second->GetBalance();
    }

    return cash_on_hand_ / liabilities;//ComputeNetAssets();
}

LoanPtr Bank::GetLoan(uint64_t number)
{
    assert(loans_.count(number) == 1);
    return loans_[number];
}

bool Bank::HasLoan(uint64_t number) const
{
    return loans_.count(number) == 1;
}

double Bank::ComputeNetAssets() const
{
    double assets = cash_on_hand_;

    for(auto account : accounts_)
    {
    //    assets -= reserve_requirement_ * account.second->GetBalance();
    }

    for(auto loan : loans_)
    {
        assets += loan.second->GetBalance();
    }
    
    return assets;
}

bool Bank::CheckWithdrawReserveRequirement(double withdraw) const
{
    double liabilities = 0.0;
    for(auto account : accounts_)
    {
        liabilities += account.second->GetBalance();
    }
    
    return (liabilities * reserve_requirement_) <= (cash_on_hand_ - withdraw);
}