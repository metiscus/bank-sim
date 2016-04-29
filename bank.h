#pragma once

#include "account.h"
#include "loan.h"
#include "isimulationentity.h"

#include <cstdint>
#include <map>
#include <memory>

class Bank : public ISimulationEntity
{
private:
    static double reserve_requirement_;
    static double base_interest_rate_;

    double cash_on_hand_;
    
    uint64_t account_number_;
    
    std::map<uint64_t, AccountPtr> accounts_;
    std::map<uint64_t, LoanPtr> loans_;

public:
    Bank(double cash_on_hand);

    static void SetReserveRequirement(double requirement);
    static void SetBaseInterestRate(double rate);
    static double GetReserveRequirement();
    static double GetBaseInterestRate();

    virtual void Simulate(double days);

    double GetCashOnHand() const;    
    void DepositCash(double cash);
    double WithdrawCash(double cash);
    
    uint64_t OpenAccount(double rate, double deposit);
    AccountPtr GetAccount(uint64_t number);
    bool HasAccount(uint64_t number) const;
    
    uint64_t OpenLoan(double rate, double amount);
    LoanPtr GetLoan(uint64_t number);
    bool HasLoan(uint64_t number) const;
    
    double ComputeNetAssets() const;
    double ComputeAssetRatio() const;

    bool CheckWithdrawReserveRequirement(double withdraw) const;
};