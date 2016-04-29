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
struct monetary_and_fiscal_policy
{
    bool central_reserve_banking;
    bool bailout_insolvent_banks;
    double target_growth_rate;
    double minimum_reserve_requirement;
    double action_lookback;
    bool can_print_money;
    bool can_change_reserve_requirement;
    uint64_t interest_rate_change_timer;
    uint64_t print_money_timer;
    uint64_t reserve_change_timer;
};

int main(int argc, char **argv)
{
    monetary_and_fiscal_policy policy;
    policy.central_reserve_banking     = true;
    policy.bailout_insolvent_banks     = true;
    policy.can_print_money             = true;
    policy.can_change_reserve_requirement = true;
    policy.target_growth_rate          = 0.03;
    policy.interest_rate_change_timer  = 120;
    policy.print_money_timer           = 180;
    policy.reserve_change_timer        = 365;
    policy.minimum_reserve_requirement = 0.05;
    policy.action_lookback             = 0.9995;

    std::vector<Bank> banks;
    banks.push_back(Bank(0));
    banks.push_back(Bank(0));
    banks.push_back(Bank(0));
    banks.push_back(Bank(0));
    banks.push_back(Bank(0));
    
    std::vector<uint64_t> accounts;
    accounts.push_back(banks[0].OpenAccount(Bank::GetBaseInterestRate(), 1e4));
    accounts.push_back(banks[1].OpenAccount(Bank::GetBaseInterestRate(), 1e4));
    accounts.push_back(banks[2].OpenAccount(Bank::GetBaseInterestRate(), 1e5));
    accounts.push_back(banks[3].OpenAccount(Bank::GetBaseInterestRate(), 1e4));
    accounts.push_back(banks[4].OpenAccount(Bank::GetBaseInterestRate(), 1e4));
    
    srand(time(0));
    
    //std::vector<LoanPtr> loans;
    std::vector< std::vector<LoanPtr> > loans;
    loans.resize(30);

    double starting_hard_assets = 0.0;    
    for(auto& bank : banks)
    {
        starting_hard_assets += bank.GetCashOnHand();
    }
    
    printf("Starting money supply: %lf\n", calculate_money_supply(banks));
    printf( "Starting hard assets: %lf\n", starting_hard_assets);
    double money_supply_old = calculate_money_supply(banks);
    for(int day = 0; day<365*20; ++day)
    {
        // time to pay loans
        std::vector<LoanPtr> remove_list;
        for(auto theLoan : loans[day%30])
        {
            if(theLoan->IsPaidOff())
            {
                continue;
            }

            uint32_t from_bank = rand() % banks.size();
            double withdraw_qty  = theLoan->GetPaymentAmount();
            double deposit_qty = banks[from_bank].GetAccount(accounts[from_bank])->Withdraw(withdraw_qty);
            if(fabs(withdraw_qty - deposit_qty) < 0.00001)
            {
                theLoan->MakePayment();
            }
            else
            {
                //fprintf(stderr, "Couldn't make payment. Returning money.\n");
                banks[from_bank].GetAccount(accounts[from_bank])->Deposit(deposit_qty);
            }
            
            if(theLoan->IsPaidOff())
            {
//                printf("Loan closed out!\n");
                remove_list.push_back(theLoan);
            }
        }

        for(auto loan : remove_list)
        {
            loans[day%30].erase( std::remove( loans[day%30].begin(), loans[day%30].end(), loan), loans[day%30].end());
        }

        // Companies will try to avoid taking out expensive loans
        // we will base the transaction frequency on how expensive the loans are
        double transaction_count = 50.0 / (Bank::GetBaseInterestRate() / 0.01 + 0.001);
        //printf("rate: %lf transaction_count: %lf\n", Bank::GetBaseInterestRate(), transaction_count);
        transaction_count = std::min(transaction_count, 1.0);
        transaction_count = std::max(transaction_count, 25.0);
        transaction_count = sqrt(transaction_count);
        for(int transactions = 0; transactions<(int)transaction_count; ++transactions)
        {
            uint32_t from_bank = rand() % banks.size();
            uint32_t to_bank   = rand() % banks.size();

            // take out a loan and deposit it
            int64_t loan_size = (rand() % 5000 + 1000.0);
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
            
//            printf("\tBank: %d Net Assets: %lf Cash: %lf Ratio: %lf\n", 
//            ++banknum, bank.ComputeNetAssets(), bank.GetCashOnHand(), bank.ComputeAssetRatio());
        }
        

        
        double money_supply = calculate_money_supply(banks);
        double growth_rate  = 365.0 * (money_supply - money_supply_old) / (money_supply_old + 0.0001);
        static double growth_rate_avg = policy.target_growth_rate;
        static uint64_t rate_timer = 30;
        static uint64_t print_timer = 30;
        static uint64_t reserve_timer = 100;
        growth_rate_avg = (1.0 - policy.action_lookback) * growth_rate + policy.action_lookback * growth_rate_avg;

        if(rate_timer > 0) --rate_timer;
        if(print_timer > 0) --print_timer;
        if(reserve_timer > 0) --reserve_timer;
        
        if(policy.central_reserve_banking && policy.target_growth_rate < 0.95 * growth_rate_avg)
        {
            // first we need to check if the money supply is constraining growth
            // if it is, we can make cash infusions to the banks to simulate "printing money"
            // we use the money multiplier theory to check this
            double theoretical_money_multiplier = 1.0 / (Bank::GetReserveRequirement());

            // compute the hard assets on hand at all banks
            double hard_assets = 0.0;
            for(auto bank : banks)
            {
                hard_assets += bank.GetCashOnHand();
            }
            
            double money_multiplier = money_supply / hard_assets;
            
            bool need_cash_infusion = (theoretical_money_multiplier - money_multiplier) < 0.05;
            if(print_timer == 0 && policy.can_print_money && need_cash_infusion)
            {
                // We "need" to infuse some money into the economy
                printf("FED: We need a cash infusion!\n");
                for(auto& bank : banks)
                {
                    bank.DepositCash(20000.0);
                }
            }
            
            if(rate_timer == 0 && Bank::GetBaseInterestRate() > 0.0)
            {
                printf("FED: Cut interest rates!\n");
                Bank::SetBaseInterestRate(
                    std::max(0.0, Bank::GetBaseInterestRate() - 0.005));
                rate_timer = policy.interest_rate_change_timer;
            }
            
            if(reserve_timer == 0 && (policy.target_growth_rate - growth_rate_avg) > 0.05  )
            {
                if(Bank::GetReserveRequirement() > policy.minimum_reserve_requirement)
                {
                    printf("FED: Decrease the reserve requirement.\n");
                    Bank::SetReserveRequirement(
                            std::max(policy.minimum_reserve_requirement,
                                     Bank::GetReserveRequirement() - 0.01));

                    reserve_timer = policy.reserve_change_timer;
                }
            }
        }
        
        if(policy.central_reserve_banking && policy.target_growth_rate + 0.025 > growth_rate_avg)
        {
            if(rate_timer == 0 && Bank::GetBaseInterestRate() < 1.0)
            {
                printf("FED: Increase interest rates!\n");
                Bank::SetBaseInterestRate(
                    std::min(1.0, Bank::GetBaseInterestRate() + 0.005));
                rate_timer = policy.interest_rate_change_timer;
            }           
        }
        

        printf("Day %d money supply: %lf growth rate: %lf\n", day, money_supply, growth_rate_avg); //100.0 * (money_supply - money_supply_old) / money_supply_old);
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
    
    printf("Final interest rate: %lf Final Reserve Requirement: %lf\n",
           Bank::GetBaseInterestRate(), Bank::GetReserveRequirement());

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