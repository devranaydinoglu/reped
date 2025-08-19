#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "text_engine.h"

class OperationTransformationTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto setupOp = std::make_unique<InsertOperation>("Hello World", 0, "c0");
        textEngine.insertIncoming(setupOp.get());
    }
    
    void TearDown() override {
        textEngine = TextEngine();
    }

    TextEngine textEngine;
};

TEST_F(OperationTransformationTest, ConcurrentInsertsEmptyDocument)
{
    TextEngine emptyEngine;
    
    // Multiple users type at position 0 of empty document
    auto op1 = std::make_unique<InsertOperation>("First", 0, "c1");
    auto op2 = std::make_unique<InsertOperation>("Second", 0, "c2");
    auto op3 = std::make_unique<InsertOperation>("Third", 0, "c3");
    
    // Transform op2 against op1
    auto transformed2 = emptyEngine.transform(op2.get(), op1.get());
    ASSERT_NE(transformed2, nullptr);
    
    // Transform op3 against op1
    auto transformed3 = emptyEngine.transform(op3.get(), op1.get());
    ASSERT_NE(transformed3, nullptr);
    
    // Verify positions are deterministically ordered by client ID
    EXPECT_EQ(transformed2->pos, 5); // "c1" < "c2", so c2 shifts right
    EXPECT_EQ(transformed3->pos, 5); // "c1" < "c3", so c3 shifts right
}

TEST_F(OperationTransformationTest, InsertAtEndWhileInsertAtBeginning)
{
    // Document: "Hello World" (length 11)
    // c1 adds "!" at end (pos 11)
    // c2 adds "Hi " at beginning (pos 0)
    
    auto c1Op = std::make_unique<InsertOperation>("!", 11, "c1");
    auto c2Op = std::make_unique<InsertOperation>("Hi ", 0, "c2");
    
    auto transformed = textEngine.transform(c1Op.get(), c2Op.get());
    
    ASSERT_NE(transformed, nullptr);
    // c1's insert should shift right by c2's insertion length
    EXPECT_EQ(transformed->pos, 14);
}

TEST_F(OperationTransformationTest, ConcurrentInsertInMiddle)
{
    // Document: "Hello World"
    // c1 inserts " beautiful" at pos 5 (after "Hello")
    // c2 inserts " wonderful" at pos 6 (after "Hello ")
    
    auto c1Op = std::make_unique<InsertOperation>(" beautiful", 5, "c1");
    auto c2Op = std::make_unique<InsertOperation>(" wonderful", 6, "c2");
    
    auto transformed = textEngine.transform(c2Op.get(), c1Op.get());
    
    ASSERT_NE(transformed, nullptr);
    // c2's insert should shift right by c1's insertion length
    EXPECT_EQ(transformed->pos, 16);
}

TEST_F(OperationTransformationTest, InsertAfterDelete)
{
    // Document: "Hello World"
    // c1 deletes "World" (pos 6, length 5)
    // c2 inserts "!" at pos 11 (at original end)
    
    auto c1Op = std::make_unique<DeleteOperation>(6, 5, "c1");
    auto c2Op = std::make_unique<InsertOperation>("!", 11, "c2");
    
    auto transformed = textEngine.transform(c2Op.get(), c1Op.get());
    
    ASSERT_NE(transformed, nullptr);
    // c2's insert should shift left by c1's deletion length
    EXPECT_EQ(transformed->pos, 6);
}

TEST_F(OperationTransformationTest, DeleteBeforeInsert)
{
    // Document: "Hello World"
    // c1 deletes "Hello " (pos 0, length 6)
    // c2 inserts "Beautiful " at pos 6 (after "Hello ")
    
    auto c1Op = std::make_unique<DeleteOperation>(0, 6, "c1");
    auto c2Op = std::make_unique<InsertOperation>("Beautiful ", 6, "c2");
    
    auto transformed = textEngine.transform(c2Op.get(), c1Op.get());
    
    ASSERT_NE(transformed, nullptr);
    // c2's insert position should move to where the deletion started
    EXPECT_EQ(transformed->pos, 0);
}

TEST_F(OperationTransformationTest, OverlappingDeletes)
{
    // Document: "Hello World"
    // c1 deletes "llo Wo" (pos 2, length 6)
    // c2 deletes "lo Wor" (pos 3, length 6)
    
    auto c1Op = std::make_unique<DeleteOperation>(2, 6, "c1");
    auto c2Op = std::make_unique<DeleteOperation>(3, 6, "c2");
    
    auto transformed = textEngine.transform(c2Op.get(), c1Op.get());
    
    if (transformed)
    {
        EXPECT_GE(transformed->pos, 0);
        EXPECT_GT(transformed->length, 0);
    }
}

TEST_F(OperationTransformationTest, SequentialTyping)
{
    TextEngine emptyEngine;
    
    // Simulate typing "Hello" one character at a time
    std::vector<std::unique_ptr<InsertOperation>> typingSequence;
    std::string word = "Hello";
    
    for (size_t i = 0; i < word.length(); i++) {
        auto op = std::make_unique<InsertOperation>(std::string(1, word[i]), i, "c1");
        op->docVersion = i;
        typingSequence.push_back(std::move(op));
    }
    
    // Another user inserts "Hi " at the beginning while c1 is typing
    auto c2Op = std::make_unique<InsertOperation>("Hi ", 0, "c2");
    c2Op->docVersion = 2; // c2's operation based on state after c1 typed "He"
    
    // Transform c1's later characters against c2's insertion
    auto transformed = emptyEngine.transform(typingSequence[3].get(), c2Op.get());
    
    ASSERT_NE(transformed, nullptr);
    EXPECT_EQ(transformed->pos, 6);
}

TEST_F(OperationTransformationTest, LargeInsertWhileEditing)
{
    // Document: "Hello World"
    // c1 does small edit: insert "," at pos 5
    // c2 pastes large text at beginning
    
    auto c1Op = std::make_unique<InsertOperation>(",", 5, "c1");
    auto c2Op = std::make_unique<InsertOperation>("This is a very long pasted text. ", 0, "c2");
    
    auto transformed = textEngine.transform(c1Op.get(), c2Op.get());
    
    ASSERT_NE(transformed, nullptr);
    EXPECT_EQ(transformed->pos, 38);
}

TEST_F(OperationTransformationTest, DeleteWhileInsert)
{
    // Document: "Hello World"
    // c1 deletes the space: delete at pos 5, length 1
    // c2 inserts "Beautiful " at pos 6 (after the space)
    
    auto c1Op = std::make_unique<DeleteOperation>(5, 1, "c1");
    auto c2Op = std::make_unique<InsertOperation>("Beautiful ", 6, "c2");
    
    auto transformed = textEngine.transform(c2Op.get(), c1Op.get());
    
    ASSERT_NE(transformed, nullptr);
    // c2's insertion should move to where c1 deleted
    EXPECT_EQ(transformed->pos, 5);
}

TEST_F(OperationTransformationTest, DocumentVersionAwareness)
{
    // Test that operations with different docVersions interact correctly
    TextEngine emptyEngine;
    
    // Op1: Insert "A" at pos 0 (based on empty doc, version 0)
    auto op1 = std::make_unique<InsertOperation>("A", 0, "c1");
    
    // Op2: Insert "B" at pos 0 (also based on empty doc, version 0)
    auto op2 = std::make_unique<InsertOperation>("B", 0, "c2");
    
    // Op3: Insert "C" at pos 1 (based on state with A, version 1)
    auto op3 = std::make_unique<InsertOperation>("C", 1, "c3");
    op3->docVersion = 1;
    
    // Transform op3 against op2 (should account for version difference)
    auto transformed = emptyEngine.transform(op3.get(), op2.get());
    
    ASSERT_NE(transformed, nullptr);
    // Since op2 happens before op3's position, shift right
    EXPECT_EQ(transformed->pos, 2);
}

TEST_F(OperationTransformationTest, ConvergenceProperty)
{
    // Apply operations in different orders, should converge to same result
    TextEngine engine1, engine2;
    
    auto opA = std::make_unique<InsertOperation>("X", 0, "c1");
    auto opB = std::make_unique<InsertOperation>("Y", 0, "c2");
    
    // Order 1: A then B
    auto opA_copy1 = std::make_unique<InsertOperation>(*opA);
    auto opB_copy1 = std::make_unique<InsertOperation>(*opB);
    engine1.insertIncoming(opA_copy1.get());
    auto transformedB = engine1.transform(opB_copy1.get(), opA_copy1.get());
    auto transformedB_insert = static_cast<InsertOperation*>(transformedB.get());
    engine1.insertIncoming(transformedB_insert);
    
    // Order 2: B then A
    auto opA_copy2 = std::make_unique<InsertOperation>(*opA);
    auto opB_copy2 = std::make_unique<InsertOperation>(*opB);
    engine2.insertIncoming(opB_copy2.get());
    auto transformedA = engine2.transform(opA_copy2.get(), opB_copy2.get());
    auto transformedA_insert = static_cast<InsertOperation*>(transformedA.get());
    engine2.insertIncoming(transformedA_insert);
    
    // Results should be identical
    EXPECT_EQ(engine1.getText(), engine2.getText());
}
