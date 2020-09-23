#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Menu.h"

using namespace std;
using namespace testing;

class menu_tests : public ::testing::Test
{
protected:
	menu_tests() {
	}

	~menu_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(menu_tests, menu_test_empty_menu_first)
{
        // Arrange
        Menu menu;

        // Act
        menu.firstMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "");
}

TEST_F(menu_tests, menu_test_empty_menu_next)
{
        // Arrange
        Menu menu;

        // Act
        menu.firstMenuItem();
        menu.nextMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "");
}

TEST_F(menu_tests, menu_test_empty_menu_prev)
{
        // Arrange
        Menu menu;

        // Act
        menu.firstMenuItem();
        menu.previousMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "");
}

TEST_F(menu_tests, menu_test_single_menu_item_first)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 0);

        // Act
        menu.firstMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu1");
}

TEST_F(menu_tests, menu_test_single_menu_item_next)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 0);

        // Act
        menu.firstMenuItem();
        menu.nextMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu1");
}

TEST_F(menu_tests, menu_test_single_menu_item_prev)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 0);

        // Act
        menu.firstMenuItem();
        menu.previousMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu1");
}

TEST_F(menu_tests, menu_test_single_menu_item_with_offset_first)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 3);

        // Act
        menu.firstMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu1");
}

TEST_F(menu_tests, menu_test_single_menu_item_with_offset_next)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 3);

        // Act
        menu.firstMenuItem();
        menu.nextMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu1");
}

TEST_F(menu_tests, menu_test_single_menu_item_with_offset_prev)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 3);

        // Act
        menu.firstMenuItem();
        menu.previousMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu1");
}

TEST_F(menu_tests, menu_test_two_menu_items_first)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 0);
        menu.setMenuItem("menu2", 1);

        // Act
        menu.firstMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu1");
}

TEST_F(menu_tests, menu_test_two_menu_items_next)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 0);
        menu.setMenuItem("menu2", 1);

        // Act
        menu.firstMenuItem();
        menu.nextMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu2");
}

TEST_F(menu_tests, menu_test_two_menu_items_prev)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 0);
        menu.setMenuItem("menu2", 1);

        // Act
        menu.firstMenuItem();
        menu.previousMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu2");
}

TEST_F(menu_tests, menu_test_two_menu_items_nextnext)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 0);
        menu.setMenuItem("menu2", 1);

        // Act
        menu.firstMenuItem();
        menu.nextMenuItem();
        menu.nextMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu1");
}

TEST_F(menu_tests, menu_test_two_menu_items_nextprev)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 0);
        menu.setMenuItem("menu2", 1);

        // Act
        menu.firstMenuItem();
        menu.nextMenuItem();
        menu.previousMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu1");
}

TEST_F(menu_tests, menu_test_two_menu_items_prevprev)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 0);
        menu.setMenuItem("menu2", 1);

        // Act
        menu.firstMenuItem();
        menu.previousMenuItem();
        menu.previousMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu1");
}

TEST_F(menu_tests, menu_test_two_menu_items_w_offset_next)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 0);
        menu.setMenuItem("menu2", 2);

        // Act
        menu.firstMenuItem();
        menu.nextMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu2");
}

TEST_F(menu_tests, menu_test_two_menu_items_w_offset_nextnext)
{
        // Arrange
        Menu menu;
        menu.setMenuItem("menu1", 0);
        menu.setMenuItem("menu2", 2);

        // Act
        menu.firstMenuItem();
        menu.nextMenuItem();
        menu.nextMenuItem();
        
        //Assert
        ASSERT_STREQ(menu.currentMenuItemName(), "menu1");
}
