#include <gtest/gtest.h>
#include <string>

#include "piece_table.h"

class PieceTableInsertStartTest : public ::testing::Test {
protected:
    void SetUp() override {
        pt.insert("Hello World", 0);
        originalText = pt.getText();
    }
    
    void TearDown() override {
        pt = PieceTable();
    }

    PieceTable pt;
    std::string originalText;
};

TEST_F(PieceTableInsertStartTest, InsertSingleCharAtStart)
{
    std::string textToInsert = "x";

    pt.insert(textToInsert, 0);
    
    std::string expected = textToInsert + originalText;
    EXPECT_EQ(pt.getText(), expected);
    EXPECT_EQ(pt.getText().size(), originalText.size() + textToInsert.size());
}

TEST_F(PieceTableInsertStartTest, InsertMultipleCharsAtStart)
{
    std::string textToInsert = "Inserted ";

    pt.insert(textToInsert, 0);
    
    std::string expected = textToInsert + originalText;
    EXPECT_EQ(pt.getText(), expected);
    EXPECT_EQ(pt.getText().size(), originalText.size() + textToInsert.size());
}

TEST_F(PieceTableInsertStartTest, InsertEmptyStringAtStart)
{
    pt.insert("", 0);
    
    EXPECT_EQ(pt.getText(), originalText);
    EXPECT_EQ(pt.getText().size(), originalText.size());
}

TEST_F(PieceTableInsertStartTest, InsertUnicodeAtStart)
{
    std::string textToInsert = "🌍";

    pt.insert(textToInsert, 0);
    
    std::string expected = textToInsert + originalText;
    EXPECT_EQ(pt.getText(), expected);
    EXPECT_EQ(pt.getText().size(), originalText.size() + textToInsert.size());
}

TEST_F(PieceTableInsertStartTest, MultipleInsertionsAtStart)
{
    pt.insert("First ", 0);
    pt.insert("Second ", 0);
    pt.insert("Third ", 0);
    
    std::string expected = "Third Second First " + originalText;
    EXPECT_EQ(pt.getText(), expected);
}
