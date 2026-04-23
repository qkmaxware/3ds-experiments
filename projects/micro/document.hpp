#include <string>
#include <vector>

#ifndef NANO_DOCUMENT
#define NANO_DOCUMENT

/// @brief A document using a piece-table editing model
class Document {
public:
    /// @brief Internal buffer type
    enum class BufferType {
        Original = 0,
        Add = 1,
    };

    /// @brief Piece table row referencing buffered content
    class Piece {
    public:
        BufferType Type;
        size_t Start;
        size_t Length;

        Piece(BufferType type, size_t start, size_t len)
            : Type(type), Start(start), Length(len) {}
    };

private:
    /// @brief Flag to indicate that the document has had some edits applied
    bool WasEdited;

    /// @brief Collection of buffers in the document
    struct BufferCollection {
        /// @brief The original document contents
        std::string original;

        /// @brief The added contents
        std::string add;

        /// @brief Fetch a buffer associated with a given piece
        /// @param piece buffer piece
        /// @return reference to the correct buffer
        inline std::string& get(const Piece& piece) {
            return get(piece.Type);
        }

        /// @brief Fetch a buffer by the buffer type
        /// @param type type of buffer
        /// @return reference to the correct buffer
        inline std::string& get(const BufferType type) {
            return (type == BufferType::Original) ? original : add;
        }
    } Buffers;

    std::vector<Piece> pieces;

    const size_t MAX_FILESIZE = 5 * 1024 * 1024; // xMB
    
public:
    Document(): WasEdited(false), Buffers{"",""}, pieces() { }

#pragma region CRUD Operations

    /// @brief Try to find the next occurrence of the given substring
    /// @param ptd_start start index in the document to begin the search
    /// @param substr the substring to find
    /// @return index to the beginning of the substring or to the end of the file if no occurrence found
    size_t FindNext(size_t ptd_start, const std::string &substr);

    /// @brief Try to open a text document at the given file path
    /// @param path file path to document
    /// @return returns true if the file was opened successfully
    bool TryOpen(const std::string &path);
    /// @brief Try to write the contents to a file at the given path
    /// @param path file path to document
    /// @return returns true if the file was written successfully
    bool TryOverwrite(const std::string &path);

    /// @brief Read characters starting at the given index into a buffer of the given length
    /// @param ptd_index start index to begin reading from
    /// @param buf buffer to read into
    /// @param length length of the buffer, max characters read
    /// @return returns the actual number of characters read
    size_t ReadChars(size_t ptd_index, char *buf, size_t length);
    /// @brief Read characters starting at the given index into a buffer of the given length stopping if the newline character is found
    /// @param ptd_index start index to begin reading from
    /// @param buf buffer to read into
    /// @param length length of the buffer, max characters read
    /// @return returns the actual number of characters read from the start index to the next newline
    size_t ReadLine(size_t ptd_index, char *buf, size_t length);

    /// @brief Insert a character into the buffer at the given index
    /// @param ptd_index index to insert the character at
    /// @param c character to insert
    void InsertAt(size_t ptd_index, char c);
    /// @brief Insert a string into the buffer at the given index
    /// @param ptd_index index to insert the characters at
    /// @param str characters to insert
    void InsertAt(size_t ptd_index, const std::string &str);
    /// @brief Replace a given segment of text with another
    /// @param ptd_index index to start replacement at
    /// @param length length of region to replace
    /// @param str the characters to use as a replacement
    void ReplaceRange(size_t ptd_index, size_t length, const std::string &str);

    /// @brief Delete the given characters from the document
    /// @param ptd_index index to start deleting at
    /// @param length number of characters to delete
    /// @return number of characters deleted
    size_t DeleteRange(size_t ptd_index, size_t length);

#pragma endregion

    /// @brief Get the total length of the document in characters
    /// @return total character count
    size_t GetLength() const;

    /// @brief Check if the document has been modified since opening
    /// @return true if document has unsaved changes
    bool IsModified() const;

    /// @brief Get a substring from the document
    /// @param start starting index
    /// @param length number of characters to extract
    /// @return the substring, or empty string if invalid range
    std::string GetSubstring(size_t start, size_t length);

    /// @brief Clear the entire document
    void Clear();

private:
    /// @brief Find which piece and offset correspond to a document index
    /// @param doc_index the document index to find
    /// @param out_piece_index where to store the piece index (set to -1 if not found)
    /// @param out_offset where to store the offset within the piece
    /// @return true if the index was found
    bool FindPieceAt(size_t doc_index, size_t& out_piece_index, size_t& out_offset) const;

};

#endif