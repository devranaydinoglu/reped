# Reped

Reped is a real-time collaborative text editor written in C++ that utilizes a client-server networking architecture to synchronize text changes across clients and Dear ImGUI to render its UI. It has a custom piece table implementation for its text buffer for efficient insertions and deletions, along with full undo/redo support.

## Text Buffer

There are various ways to implement a text buffer. It can use a naive approach with a string or array, or more efficient and robust data structures that have already been tried-and-tested in existing editors. Below I will briefly explain how gap buffers, ropes, and piece tables (my choice), examples of efficient and robust data structures that have already been tried-and-tested in existing editors, work.

There are several ways to implement a text buffer for a text editor. A naïve yet simple approach is to use a string or array, but more efficient and reliable methods exist. Below, I’ll briefly explain how gap buffers, ropes, and piece tables (my preferred choice) work, all of which are well-tested and efficient data structures used in established text/code editors.

### Gap Buffer

A gap buffer is a data structure which is commonly used in text editors to represent a text document. It works by splitting text into two contiguous segments with a gap between them. This gap represents the location of the cursor. It is built around text operations at or near the cursor position. Moving the cursor involves copying text from one segment to the other. A gap buffer becomes inefficient when text operations are done at different locations in the text (e.g. multi-cursor edits) or when inserting large texts that fill the gap. The latter involves creating a new gap which requires copying large portions of the text. Deleting a character involves moving the gap to the character's position and extending it to cover the deleted character.

Since Reped is a collaborative text editor with multiple clients modifying the document, text is inserted and deleted throughout the document in different locations. This makes the gap buffer not ideal when receiving operations from multiple clients that need to be applied to a local document.

<img width="417" height="208" alt="Screenshot 2025-07-27 at 12 32 06" src="https://github.com/user-attachments/assets/effc7026-f5c3-4979-9c56-cff5afc23cbe" />

### Rope

A rope data structure is a binary tree where each leaf is immutable and holds a text segment and its length as the weight. Parent nodes hold the sum of the lengths of all the leaves in its left subtree.

Reped applies insertions and deletions per character. This means that it has to update the text buffer frequently. Insertions and deletions in a rope can require splitting the tree, rebalancing the tree, and reconstructing new subtrees. It's more efficient than a gap buffer but there is still room for improvement.

<img width="452" height="208" alt="Screenshot 2025-07-27 at 12 36 24" src="https://github.com/user-attachments/assets/03ab6cb9-c195-40a9-9f6b-53580ebdf96d" />

### Piece Table

A piece table works by having a buffer file containing the contents of the original, unchanged state of a text file. This buffer is read-only. Then there is a second "add" buffer file that is append-only, containing all the newly added text to the file in a session.

A piece table row can be referred to as a piece and acts as a pointer to a span. A span is a string of items that is physically contiguous in a buffer and is also logically contiguous in the text sequence. A piece consists of three columns: the buffer, start index of the text segment in the buffer, length of the text segment in the buffer. When a file is opened intially, there is only one piece which points to the entire original buffer.

A deletion involves splitting a span into two spans. One of the spans points to the items in the old span up to the deleted item and the other span points to the items after the deleted item. When a deleted item is at the start or end of a span, the start index or length in the piece is adjusted.

An insertion is handled by splitting the span into three spans. The first span points to the items of the old span up to the inserted item. The third span points to the items of the old span after the inserted item. The inserted item is appended to the end of the add buffer file and the second span points to the inserted item. It's also possible to combine multiple insertions in succession into a single span.

<img width="640" height="291" alt="Screenshot 2025-07-27 at 12 38 03" src="https://github.com/user-attachments/assets/488722eb-ea58-4cce-b300-ba6703e968d3" />
