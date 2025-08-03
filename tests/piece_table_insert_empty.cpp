#include <gtest/gtest.h>

#include "piece_table.h"

class PieceTableInsertEmptyTest : public ::testing::Test {
protected:
    PieceTable pt;
};

TEST_F(PieceTableInsertEmptyTest, InsertSingleChar)
{
    std::string insertedText = "a";

    pt.insert(insertedText, 0);

    std::string newText = pt.getText();
    EXPECT_EQ(newText.size(), 1);
    EXPECT_EQ(newText, insertedText);    
    EXPECT_FALSE(newText.empty());
    EXPECT_EQ(newText[0], insertedText[0]);
}

TEST_F(PieceTableInsertEmptyTest, InsertMultipleChars)
{
    std::string insertedText = "hello";
    
    pt.insert(insertedText, 0);
    
    std::string newText = pt.getText();
    EXPECT_EQ(newText.size(), 5);
    EXPECT_EQ(newText, insertedText);
}

TEST_F(PieceTableInsertEmptyTest, InsertUnicode)
{
    std::string insertedText = "世界";
    
    pt.insert(insertedText, 0);
    
    std::string newText = pt.getText();
    EXPECT_EQ(newText, insertedText);
    EXPECT_EQ(newText.size(), insertedText.size());
}

TEST_F(PieceTableInsertEmptyTest, InsertAtNonZeroIndex)
{
    std::string insertedText = "hello";

    pt.insert(insertedText, 5);
    
    std::string newText = pt.getText();
    EXPECT_EQ(newText, insertedText);
    EXPECT_EQ(newText.size(), insertedText.size());
}

TEST_F(PieceTableInsertEmptyTest, InsertEmptyString)
{
    pt.insert("", 0);
    
    std::string newText = pt.getText();
    EXPECT_EQ(newText, "");
    EXPECT_EQ(newText.size(), 0);
    EXPECT_TRUE(newText.empty());
}

TEST_F(PieceTableInsertEmptyTest, InsertSpecialCharacters)
{
    std::string insertedText = "\n\t\r";
    
    pt.insert(insertedText, 0);
    
    std::string newText = pt.getText();
    EXPECT_EQ(newText, insertedText);
    EXPECT_EQ(newText.size(), insertedText.size());
}

TEST_F(PieceTableInsertEmptyTest, InsertLargeText)
{
    std::string insertedText(1000, 'X');
    
    pt.insert(insertedText, 0);
    
    std::string newText = pt.getText();
    EXPECT_EQ(newText, insertedText);
    EXPECT_EQ(newText.size(), insertedText.size());
}
