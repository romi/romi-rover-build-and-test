#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ScriptList.h"
#include "ScriptMenu.h"

using namespace std;
using namespace testing;
using namespace romi;

class scriptmenu_tests : public ::testing::Test
{
protected:
        JsonCpp json;
        
	scriptmenu_tests() {
                json = JsonCpp::parse("[{ 'id': 'foo', 'title': 'Foo', "
                                      "  'actions': [{'action':'move', 'distance': 1, 'speed': 0.5}]},"
                                      "{ 'id': 'bar', 'title': 'Bar', "
                                      "  'actions': [{'action':'hoe'}]}]");
	}

	~scriptmenu_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(scriptmenu_tests, test_constructor)
{
        ScriptList scripts(json);
        ScriptMenu menu(scripts);
}

TEST_F(scriptmenu_tests, test_empty_menu_1)
{
        JsonCpp empty = JsonCpp::parse("[]");
        ScriptList scripts(empty);
        ScriptMenu menu(scripts);
        std::string title;

        menu.first_menu_item(title);
        
        ASSERT_STREQ(title.c_str(), Menu::Empty);
        ASSERT_EQ(menu.get_current_index(), -1);
}

TEST_F(scriptmenu_tests, test_empty_menu_2)
{
        JsonCpp empty = JsonCpp::parse("[]");
        ScriptList scripts(empty);
        ScriptMenu menu(scripts);
        std::string title;

        menu.first_menu_item(title);
        menu.next_menu_item(title);
        
        ASSERT_STREQ(title.c_str(), Menu::Empty);
        ASSERT_EQ(menu.get_current_index(), -1);
}

TEST_F(scriptmenu_tests, test_empty_menu_3)
{
        JsonCpp empty = JsonCpp::parse("[]");
        ScriptList scripts(empty);
        ScriptMenu menu(scripts);
        std::string title;

        menu.first_menu_item(title);
        menu.previous_menu_item(title);
        
        ASSERT_STREQ(title.c_str(), Menu::Empty);
        ASSERT_EQ(menu.get_current_index(), -1);
}

TEST_F(scriptmenu_tests, test_first_menu)
{
        ScriptList scripts(json);
        ScriptMenu menu(scripts);
        std::string title;
        
        menu.first_menu_item(title);
        
        ASSERT_STREQ(title.c_str(), "Foo");
        ASSERT_EQ(menu.get_current_index(), 0);
}

TEST_F(scriptmenu_tests, test_second_menu)
{
        ScriptList scripts(json);
        ScriptMenu menu(scripts);
        std::string title;
        
        menu.first_menu_item(title);
        menu.next_menu_item(title);
        
        ASSERT_STREQ(title.c_str(), "Bar");
        ASSERT_EQ(menu.get_current_index(), 1);
}

TEST_F(scriptmenu_tests, test_third_menu)
{
        ScriptList scripts(json);
        ScriptMenu menu(scripts);
        std::string title;
        
        menu.first_menu_item(title);
        menu.next_menu_item(title);
        menu.next_menu_item(title);
        
        ASSERT_STREQ(title.c_str(), "Bar");
        ASSERT_EQ(menu.get_current_index(), 1);
}

TEST_F(scriptmenu_tests, test_previous_menu)
{
        ScriptList scripts(json);
        ScriptMenu menu(scripts);
        std::string title;
        
        menu.first_menu_item(title);
        menu.previous_menu_item(title);
        
        ASSERT_STREQ(title.c_str(), "Foo");
        ASSERT_EQ(menu.get_current_index(), 0);
}

TEST_F(scriptmenu_tests, test_next_then_previous_menu)
{
        ScriptList scripts(json);
        ScriptMenu menu(scripts);
        std::string title;
        
        menu.first_menu_item(title);
        menu.next_menu_item(title);
        menu.previous_menu_item(title);
        
        ASSERT_STREQ(title.c_str(), "Foo");
        ASSERT_EQ(menu.get_current_index(), 0);
}
