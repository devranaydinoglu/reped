#include <gtest/gtest.h>

#include "piece_table.h"

class PieceTableInsertEndTest : public ::testing::Test {
protected:
    void SetUp() override {
        pt.insert("Hello World", 0);
        originalText = pt.getText();
        endIndex = originalText.size();
    }
    
    void TearDown() override {
        pt = PieceTable();
    }

    PieceTable pt;
    std::string originalText;
    std::size_t endIndex;
};

TEST_F(PieceTableInsertEndTest, InsertSingleCharAtEnd)
{
    std::string insertedText = "!";

    pt.insert(insertedText, endIndex);
    
    std::string expected = originalText + insertedText;
    EXPECT_EQ(pt.getText(), expected);
    EXPECT_EQ(pt.getText().size(), originalText.size() + insertedText.size());
}

TEST_F(PieceTableInsertEndTest, InsertMultipleCharsAtEnd)
{
    std::string insertedText = " - The End";

    pt.insert(insertedText, endIndex);
    
    std::string expected = originalText + insertedText;
    EXPECT_EQ(pt.getText(), expected);
    EXPECT_EQ(pt.getText().size(), originalText.size() + insertedText.size());
}

TEST_F(PieceTableInsertEndTest, InsertEmptyStringAtEnd)
{
    std::string beforeInsert = pt.getText();

    pt.insert("", endIndex);
    
    EXPECT_EQ(pt.getText(), beforeInsert);
    EXPECT_EQ(pt.getText().size(), beforeInsert.size());
}

TEST_F(PieceTableInsertEndTest, InsertUnicodeAtEnd)
{
    std::string insertedText = "🌍";

    pt.insert(insertedText, endIndex);
    
    std::string expected = originalText + insertedText;
    EXPECT_EQ(pt.getText(), expected);
    EXPECT_EQ(pt.getText().size(), originalText.size() + insertedText.size());
}

TEST_F(PieceTableInsertEndTest, MultipleInsertionsAtEnd)
{
    pt.insert(" First", endIndex);
    pt.insert(" Second", pt.getText().size());
    pt.insert(" Third", pt.getText().size());
    
    std::string expected = originalText + " First Second Third";
    EXPECT_EQ(pt.getText(), expected);
}

TEST_F(PieceTableInsertEndTest, InsertBeyondEndIndex)
{
    std::string insertedText = "appended";
    std::size_t beyondIndex = endIndex + 100;

    pt.insert(insertedText, beyondIndex);
    
    std::string expected = originalText + insertedText;
    EXPECT_EQ(pt.getText(), expected);
}

TEST_F(PieceTableInsertEndTest, InsertSpecialCharactersAtEnd)
{
    std::string insertedText = "\n\t\r";
    pt.insert(insertedText, endIndex);
    
    std::string expected = originalText + insertedText;
    EXPECT_EQ(pt.getText(), expected);
    EXPECT_EQ(pt.getText().size(), originalText.size() + insertedText.size());
}

TEST_F(PieceTableInsertEndTest, InsertLargeTextAtEnd)
{
    std::string largeText(1000, 'X');
    pt.insert(largeText, endIndex);
    
    std::string expected = originalText + largeText;
    EXPECT_EQ(pt.getText(), expected);
    EXPECT_EQ(pt.getText().size(), originalText.size() + 1000);
}
