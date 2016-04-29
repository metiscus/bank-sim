#include "account.h"
#include "bank.h"

Account::Account(Bank& bank, double balance, double annual_rate)
    : bank_(bank)
    , balance_(balance)
    , annual_rate_(annual_rate)
{
    bank_.DepositCash(balance);
}

double Account::GetBalance() const
{
    return balance_;
}

void Account::Deposit(double value)
{
    balance_ += value;
    bank_.DepositCash(value);
}

double Account::Withdraw(double value)
{
    double withdrawn = value;
    
    // ensure that the bank has the cash on hand
    withdrawn = std::min(value, bank_.GetCashOnHand());
    withdrawn = std::min(withdrawn, balance_);

    if(balance_ <= withdrawn)
    {
        withdrawn = balance_;
        balance_ = 0.0f;
    }
    else
    {
        balance_ -= value;
    }
    
    withdrawn = bank_.WithdrawCash(withdrawn);

    return withdrawn;
}

void Account::Simulate(double days)
{
    balance_ += balance_ * annual_rate_ / 365.0 *days;
}