#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "piece_table.h"

class PieceTableInsertMiddleTest : public ::testing::Test {
protected:
    void SetUp() override {
        pt.insert("Hello World", 0);
        originalText = pt.getText();
        middleIndex = originalText.size() / 2;
    }
    
    void TearDown() override {
        pt = PieceTable();
    }
    
    PieceTable pt;
    std::string originalText;
    std::size_t middleIndex;
};

TEST_F(PieceTableInsertMiddleTest, InsertAtExactMiddle)
{
    std::string insertedText = "inserted";

    pt.insert(insertedText, middleIndex);
    
    std::string expected = originalText.substr(0, middleIndex) + insertedText + 
                            originalText.substr(middleIndex);
    EXPECT_EQ(pt.getText(), expected);
    EXPECT_EQ(pt.getText().size(), originalText.size() + insertedText.size());
}

TEST_F(PieceTableInsertMiddleTest, InsertAtDifferentPositions)
{
    std::string insertedTextQuarter = "quarter";
    std::string insertedTextThreeQuarter = "three-quarter";

    pt.insert(insertedTextQuarter, originalText.size() / 4);
    pt.insert(insertedTextThreeQuarter, 3 * originalText.size() / 4);
    
    std::string result = pt.getText();
    EXPECT_GT(result.size(), originalText.size());
    EXPECT_THAT(result, ::testing::HasSubstr(insertedTextQuarter));
    EXPECT_THAT(result, ::testing::HasSubstr(insertedTextThreeQuarter));
}

TEST_F(PieceTableInsertMiddleTest, InsertAtBoundaryIndices)
{
    std::string insertedTextNearStart = "a";
    std::string insertedTextNearEnd = "z";

    pt.insert(insertedTextNearStart, 1);
    pt.insert(insertedTextNearEnd, pt.getText().size() - 1);

    std::string expected = originalText.insert(1, insertedTextNearStart).insert(originalText.size() - 1, insertedTextNearEnd);
    EXPECT_EQ(pt.getText()[1], insertedTextNearStart[0]);    
    EXPECT_EQ(pt.getText()[pt.getText().size() - 2], insertedTextNearEnd[0]);
    EXPECT_EQ(pt.getText(), expected);
}

TEST_F(PieceTableInsertMiddleTest, InsertEmptyStringInMiddle)
{
    std::string beforeInsert = pt.getText();

    pt.insert("", middleIndex);
    
    EXPECT_EQ(pt.getText(), beforeInsert);
    EXPECT_EQ(pt.getText().size(), beforeInsert.size());
}

TEST_F(PieceTableInsertMiddleTest, InsertBeyondDocumentLength)
{
    std::string insertedText = "append";
    std::size_t beyondIndex = originalText.size() + 100;

    pt.insert(insertedText, beyondIndex);
    
    std::string expected = originalText + insertedText;
    EXPECT_EQ(pt.getText(), expected);
}

TEST_F(PieceTableInsertMiddleTest, MultipleInsertionsInMiddle)
{
    std::string insertedTextFirst = "first";
    std::string insertedTextSecond = "second";
    std::string insertedTextThird = "third";

    pt.insert(insertedTextFirst, 1);
    pt.insert(insertedTextSecond, pt.getText().size() - 1);
    pt.insert(insertedTextThird, pt.getText().size() / 2);
    
    std::string result = pt.getText();
    EXPECT_THAT(result, ::testing::HasSubstr(insertedTextFirst));
    EXPECT_THAT(result, ::testing::HasSubstr(insertedTextSecond));
    EXPECT_THAT(result, ::testing::HasSubstr(insertedTextThird));
}

TEST_F(PieceTableInsertMiddleTest, InsertUnicodeInMiddle)
{
    std::string insertedText = "世界";

    pt.insert(insertedText, middleIndex);
    
    std::string result = pt.getText();
    EXPECT_THAT(result, ::testing::HasSubstr(insertedText));
    EXPECT_EQ(result.size(), originalText.size() + insertedText.size());
}
