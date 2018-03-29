#include <list>
#include <utility>
#include <iostream>

#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>

#include "cron_parser.hpp"


int main()
{
    try {
        sched::Schedule schedule = sched::parse_cron("0 0,15,30,45 * 1-30/2 JUN-AUG,DEC-FEB MON-FRI");
        
        auto field_to_string = [] (const sched::Field& field) {
        std::list<std::string> items;

            for (auto& item: field) {
                items.push_back((boost::format("%1%-%2%:%3%") 
                    % +std::get<0>(item)
                    % +std::get<1>(item)
                    % +std::get<2>(item)
                ).str());
            }  

            return boost::algorithm::join(items, ", ");              
        };

        std::cout << (boost::format(
            "seconds:       %1%\n"
            "minutes:       %2%\n"
            "hours:         %3%\n"
            "day of month:  %4%\n"
            "month:         %5%\n"
            "day of week:   %6%\n"
        ) % field_to_string(std::get<0>(schedule))
          % field_to_string(std::get<1>(schedule))
          % field_to_string(std::get<2>(schedule))
          % field_to_string(std::get<3>(schedule))
          % field_to_string(std::get<4>(schedule))
          % field_to_string(std::get<5>(schedule))
        ).str();
    }
    catch (std::invalid_argument& e) {
        std::cerr << "invalid cron expression: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}