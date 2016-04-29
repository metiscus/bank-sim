#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <map>
#include <vector>

#include "isimulationentity.h"
#include "bank.h"
#include "loan.h"

double calculate_money_supply(std::vector<Bank>& banks);

int main(int argc, char **argv)
{
    std::vector<Bank> banks;
    banks.push_back(Bank(0));
    banks.push_back(Bank(0));
    banks.push_back(Bank(0));
    banks.push_back(Bank(0));
    banks.push_back(Bank(0));
    
    double starting_hard_assets = 0.0;
    std::vector<uint64_t> accounts;
    accounts.push_back(banks[0].OpenAccount(Bank::GetBaseInterestRate(), 1e5));
    accounts.push_back(banks[1].OpenAccount(Bank::GetBaseInterestRate(), 1e5));
    accounts.push_back(banks[2].OpenAccount(Bank::GetBaseInterestRate(), 1e5));
    accounts.push_back(banks[3].OpenAccount(Bank::GetBaseInterestRate(), 1e5));
    accounts.push_back(banks[4].OpenAccount(Bank::GetBaseInterestRate(), 1e5));
    
    srand(time(0));
    
    //std::vector<LoanPtr> loans;
    std::vector< std::vector<LoanPtr> > loans;
    loans.resize(30);
    
    for(auto& bank : banks)
    {
        starting_hard_assets += bank.GetCashOnHand();
    }
    
    printf("Starting money supply: %lf\n", calculate_money_supply(banks));
    printf( "Starting hard assets: %lf\n", starting_hard_assets);
    double money_supply_old = calculate_money_supply(banks);
    for(int day = 0; day<365*20; ++day)
    {
#if 0
        double hard_assets_old = 0.0;
        for(auto bank : banks)
        {
            hard_assets_old += bank.GetCashOnHand();
        }
#endif

        //if(day % 30 == 0)
        {
            std::vector< std::vector<LoanPtr> > newLoans;
            newLoans.resize(loans.size());

            // time to pay loans
            for(auto theLoan : loans[day%30])
            {
                if(theLoan->IsPaidOff())
                {
                    continue;
                }

                uint32_t from_bank = rand() % banks.size();
                double withdraw_qty  = theLoan->GetPaymentAmount();
                double deposit_qty = banks[from_bank].GetAccount(accounts[from_bank])->Withdraw(withdraw_qty);
                if(fabs(withdraw_qty - deposit_qty) < 0.0001)
                {
                    theLoan->MakePayment();
                }
                else
                {
                    fprintf(stderr, "Couldn't make payment. Returning money.\n");
                    banks[from_bank].GetAccount(accounts[from_bank])->Deposit(deposit_qty);
                }
                
                if(!theLoan->IsPaidOff())
                {
                    newLoans[day%30].push_back(theLoan);
                }
            }
            
            loans = newLoans;
        }

        if(0 && day == 3650)
        {
            for(auto& bank : banks)
            {
                bank.DepositCash(1e5);
            }
        }
        

        for(int transactions = 0; transactions<25; ++transactions)
        {
            uint32_t from_bank = rand() % banks.size();
            uint32_t to_bank   = rand() % banks.size();

            // take out a loan and deposit it
            int64_t loan_size = rand() % 500 + 100.0;
            uint64_t loan_acct = banks[from_bank].OpenLoan(Bank::GetBaseInterestRate() + 0.025, loan_size);
            if(loan_acct != 0)
            {
                banks[to_bank].GetAccount(accounts[to_bank])->Deposit(loan_size);
                loans[day%30].push_back(banks[from_bank].GetLoan(loan_acct));
            }
        }
        
        int banknum = 0;
        for(auto& bank : banks)
        {
            bank.Simulate(1);
            
            printf("\tBank: %d Net Assets: %lf Cash: %lf Ratio: %lf\n", 
            ++banknum, bank.ComputeNetAssets(), bank.GetCashOnHand(), bank.ComputeAssetRatio());
        }
        
#if 0
        double hard_assets = 0.0;
        for(auto bank : banks)
        {
            hard_assets += bank.GetCashOnHand();
        }         
        
        if(fabs(hard_assets-hard_assets_old)> 0.01)
        {
            fprintf(stderr, "error: capital growth detected!\n");
        }
#endif
        
        double money_supply = calculate_money_supply(banks);
        printf("Day %d money supply: %lf growth rate: %lf\n", day, money_supply, 365.0 * 100.0 * (money_supply - money_supply_old) / money_supply_old);
        money_supply_old = money_supply;

    }
    money_supply_old = calculate_money_supply(banks);
    double hard_assets = 0.0;
    for(auto bank : banks)
    {
        hard_assets += bank.GetCashOnHand();
    }
    printf("End of simulation:\nMoney Supply: %lf\nHard Assets: %lf\nAsset Ratio: %lf\n",
           money_supply_old, hard_assets, money_supply_old / hard_assets);
    
    return 0;
}

double calculate_money_supply(std::vector<Bank>& banks)
{
    double money_supply = 0;
    
    for(auto bank : banks)
    {
        money_supply += bank.ComputeNetAssets();
    }
    
    return money_supply;
}