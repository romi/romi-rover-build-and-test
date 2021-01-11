#include "gmock/gmock.h"
#include "Menu.h"

class MockMenu : public romi::Menu
{
public:
        MOCK_METHOD(void, first_menu_item, (std::string& name), (override));
        MOCK_METHOD(void, next_menu_item, (std::string& name), (override));
        MOCK_METHOD(void, previous_menu_item, (std::string& name), (override));
        MOCK_METHOD(void, get_current_menu, (std::string& name), (override));
        MOCK_METHOD(int, get_current_index, (), (override));
};
