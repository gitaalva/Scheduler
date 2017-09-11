

//Define our Module name (prints at testing)
 #define BOOST_TEST_MODULE "BaseClassModule"

//VERY IMPORTANT - include this last
#include <boost/test/unit_test.hpp>
#include <iostream>

// ------------- Tests Follow --------------
BOOST_AUTO_TEST_CASE( constructors )
{
    std::cout << "Does nothing as of now" << std::endl;
}
