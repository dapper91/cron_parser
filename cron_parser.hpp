#include <vector>
#include <string>
#include <iterator>
#include <stdexcept>
#include <type_traits>

#include <boost/format.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/phoenix/bind/bind_member_function.hpp>
#include <boost/phoenix/statement/throw.hpp>


namespace sched {


namespace qi = boost::spirit::qi;


typedef std::tuple<uint8_t, uint8_t, uint8_t> Item;
typedef std::vector<Item> Field;
typedef std::tuple<Field,Field,Field,Field,Field,Field> Schedule;


template <typename Iterator = std::string::const_iterator>
struct CronGrammar: qi::grammar<Iterator, Schedule()> {    

    qi::symbols<char, uint16_t> month_, day_of_week_;

    qi::rule<Iterator, uint8_t()> second_value,
                                  minute_value,
                                  hour_value,
                                  day_of_month_value,
                                  month_value,
                                  day_of_week_value,
                                  step;

    qi::rule<Iterator, Item(), qi::locals<uint8_t, uint8_t, uint8_t>> second_item,
                                                                      minute_item,
                                                                      hour_item,
                                                                      day_of_month_item,
                                                                      month_item,
                                                                      day_of_week_item;

    qi::rule<Iterator, Field()> second_field,
                                minute_field,
                                hour_field,
                                day_of_month_field,
                                month_field,
                                day_of_week_field;

    qi::rule<Iterator, Schedule()> schedule;


    CronGrammar(): CronGrammar::base_type(schedule, "cron grammar"),        
        second_value(std::string("second value")),
        minute_value(std::string("minute value")),
        hour_value(std::string("hour value")),
        day_of_month_value(std::string("day of month value")),
        month_value(std::string("month value")),        
        day_of_week_value(std::string("day of week value")),
        step(std::string("step value")),

        second_item(std::string("second item")),
        minute_item(std::string("minute item")),
        hour_item(std::string("hour item")),
        day_of_month_item(std::string("day of month item")),
        month_item(std::string("month item")),
        day_of_week_item(std::string("day of week item")),

        second_field(std::string("second field")),
        minute_field(std::string("minute field")),
        hour_field(std::string("hour field")),
        day_of_month_field(std::string("day of month field")),
        month_field(std::string("month field")),
        day_of_week_field(std::string("day of week field"))
    {
        using qi::on_error;
        using qi::fail;        
        using namespace qi::labels;
        using boost::spirit::ushort_;
        using boost::spirit::ascii::char_;
        using boost::spirit::ascii::space;
        using boost::spirit::omit;        
        using boost::spirit::eoi;
        using boost::spirit::eps;
        using boost::phoenix::push_back;
        using boost::phoenix::construct;
        using boost::phoenix::throw_;

        month_.add
            ("JAN",  1)
            ("FEB",  2)
            ("MAR",  3)
            ("APR",  4)
            ("MAY",  5)
            ("JUN",  6)
            ("JUL",  7)
            ("AUG",  8)
            ("SEP",  9)
            ("OCT", 10)
            ("NOV", 11)
            ("DEC", 12)
        ;

        day_of_week_.add
            ("SUN", 0)
            ("MON", 1)
            ("TUE", 2)
            ("WED", 3)
            ("THU", 4)
            ("FRI", 5)
            ("SAT", 6)
        ;

        second_value        %= ushort_                  [_pass = (0 <= _1 && _1 <= 59)];

        minute_value        %= ushort_                  [_pass = (0 <= _1 && _1 <= 59)];

        hour_value          %= ushort_                  [_pass = (0 <= _1 && _1 <= 23)];

        day_of_month_value  %= ushort_                  [_pass = (1 <= _1 && _1 <= 31)];

        month_value         %= ushort_ | month_         [_pass = (1 <= _1 && _1 <= 12)];

        day_of_week_value   %= ushort_ | day_of_week_   [_pass = (0 <= _1 && _1 <= 06)];

        step                %= ushort_;

        
        auto define_field_rule = [&] (auto& field, auto& item, auto& value) {
        
        item =
        (
            (
                (value >> '-' > value)          [_a = _1, _b = _2, _c = 1]
              | value                           [_a = _1, _b = _1, _c = 1]
              | char_('*')                      [_a = -1, _b = -1, _c = 1]
            )  >>
           -(
                '/' > step                      [_c = _1]
            )
        )                                       [_val = construct<Item>(_a, _b, _c)];

        field = item                            [push_back(_val, _1)]
              % ',';
        
        };

        define_field_rule(second_field,         second_item,        second_value);
        define_field_rule(minute_field,         minute_item,        minute_value);
        define_field_rule(hour_field,           hour_item,          hour_value);
        define_field_rule(day_of_month_field,   day_of_month_item,  day_of_month_value);
        define_field_rule(month_field,          month_item,         month_value);
        define_field_rule(day_of_week_field,    day_of_week_item,   day_of_week_value);
          
        
        schedule = eps >
        (
            second_field          > omit[space] > 
            minute_field          > omit[space] > 
            hour_field            > omit[space] > 
            day_of_month_field    > omit[space] > 
            month_field           > omit[space] > 
            day_of_week_field     > omit[eoi]
        )                                           [_val = construct<Schedule>(_1, _2, _3, _4, _5, _6)];


        on_error<fail>(schedule, boost::phoenix::bind(&CronGrammar::error_handler, this, _1, _2, _3, _4));
    }

    void error_handler(const Iterator first, const Iterator last, const Iterator error_pos, const boost::spirit::info& what)
    {
        std::string tag = what.tag;
        if (tag == "omit") {
            tag = boost::get<boost::spirit::info>(what.value).tag;
        }

        throw std::invalid_argument((boost::format("expecting '%1%' at position %2%") % tag % (std::distance(first, error_pos) + 1)).str());
    }
};


Schedule parse_cron(const std::string& cron_str)
{
    sched::CronGrammar grammar;
    sched::Schedule schedule;   

    auto begin = std::begin(cron_str), end = std::end(cron_str);

    if (!parse(begin, end, grammar, schedule)) {
        std::invalid_argument((boost::format("parsing error at position %1%") % (std::distance(std::begin(cron_str), begin) + 1)).str());
    }

    return std::move(schedule); 
}


} // namespace sched
