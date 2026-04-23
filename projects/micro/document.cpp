#include "document.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>

// Helper method to find which piece contains a given document index
bool Document::FindPieceAt(size_t doc_index, size_t& out_piece_index, size_t& out_offset) const {
    size_t current_pos = 0;
    
    for (size_t i = 0; i < pieces.size(); ++i) {
        if (current_pos + pieces[i].Length > doc_index) {
            out_piece_index = i;
            out_offset = doc_index - current_pos;
            return true;
        }
        current_pos += pieces[i].Length;
    }
    
    return false;
}

size_t Document::FindNext(size_t ptd_start, const std::string &substr) {
    if (substr.empty()) {
        return ptd_start;
    }

    size_t doc_len = GetLength();
    if (ptd_start >= doc_len) {
        return doc_len; // No match, at or past end
    }

    // Allocate buffer for comparison
    char read_buffer[512];
    size_t substr_len = substr.length();
    size_t search_pos = ptd_start;

    // Search through the document
    while (search_pos <= doc_len - substr_len) {
        // Read the potential match
        size_t read_len = ReadChars(search_pos, read_buffer, substr_len);

        // If we couldn't read enough characters, we've reached end of document
        if (read_len < substr_len) {
            return doc_len;
        }

        // Compare the read characters with the substring
        if (std::memcmp(read_buffer, substr.c_str(), substr_len) == 0) {
            return search_pos; // Match found
        }

        search_pos++;
    }

    return doc_len; // No match found
}

bool Document::TryOpen(const std::string &path) {
    // Try to open the file
    FILE *f = fopen(path.c_str(), "rb");
    if (!f) return false;

    // Determine file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size < 0 || (size_t)size > MAX_FILESIZE) {
        fclose(f);
        return false;
    }

    // Read the document into the buffer
    Buffers.original.resize(size);
    fread(&Buffers.original[0], 1, size, f);
    fclose(f);

    // Clear the add buffer
    Buffers.add.clear();

    // Create a piece
    pieces.clear();
    if (size > 0) {
        pieces.push_back(Document::Piece(BufferType::Original, 0, (size_t)size));
    }

    // Mark as not edited
    WasEdited = false;

    // Indicate a successful load
    return true;
}

bool Document::TryOverwrite(const std::string &path) {
    if (!WasEdited) {
        // Saving no changes, who cares
        return true;
    }

    // Try to open the file
    FILE *f = fopen(path.c_str(), "wb");
    if (!f) return false;

    // Copy pieces from buffer
    for (const Piece& piece : pieces) {
        const std::string& buffer = Buffers.get(piece);
        fwrite(buffer.data() + piece.Start, 1, piece.Length, f);
    }

    // Indicate a successful write
    fclose(f);
    return true;
}

size_t Document::GetLength() const {
    size_t total = 0;
    for (const Piece& piece : pieces) {
        total += piece.Length;
    }
    return total;
}

bool Document::IsModified() const {
    return WasEdited;
}

std::string Document::GetSubstring(size_t start, size_t length) {
    std::string result;
    char buffer[4096];
    
    size_t remaining = length;
    size_t pos = start;
    
    while (remaining > 0) {
        size_t to_read = (remaining > sizeof(buffer)) ? sizeof(buffer) : remaining;
        size_t read = ReadChars(pos, buffer, to_read);
        
        if (read == 0) break;
        
        result.append(buffer, read);
        pos += read;
        remaining -= read;
    }
    
    return result;
}

void Document::Clear() {
    Buffers.original.clear();
    Buffers.add.clear();
    pieces.clear();
    WasEdited = true;
}

size_t Document::ReadChars(size_t ptd_index, char *buf, size_t length) {
    if (length == 0 || buf == nullptr) return 0;
    
    size_t piece_index;
    size_t offset;
    
    // Find starting piece
    if (!FindPieceAt(ptd_index, piece_index, offset)) {
        return 0; // Index out of range
    }
    
    size_t read_count = 0;
    
    // Read from current and subsequent pieces
    while (read_count < length && piece_index < pieces.size()) {
        const Piece& piece = pieces[piece_index];
        const std::string& buffer = Buffers.get(piece);
        
        // How much can we read from this piece?
        size_t available = piece.Length - offset;
        size_t to_read = (available < (length - read_count)) ? available : (length - read_count);
        
        // Copy data
        if (to_read > 0) {
            std::memcpy(buf + read_count, buffer.data() + piece.Start + offset, to_read);
            read_count += to_read;
        }
        
        // Move to next piece
        piece_index++;
        offset = 0;
    }
    
    return read_count;
}

size_t Document::ReadLine(size_t ptd_index, char *buf, size_t length) {
    if (length == 0 || buf == nullptr) return 0;
    
    size_t piece_index;
    size_t offset;
    
    // Find starting piece
    if (!FindPieceAt(ptd_index, piece_index, offset)) {
        return 0; // Index out of range
    }
    
    size_t read_count = 0;
    
    // Read until newline or buffer full
    while (read_count < length && piece_index < pieces.size()) {
        const Piece& piece = pieces[piece_index];
        const std::string& buffer = Buffers.get(piece);
        
        // Search for newline in this piece
        for (size_t i = offset; i < piece.Length && read_count < length; ++i) {
            buf[read_count++] = buffer[piece.Start + i];
            
            if (buffer[piece.Start + i] == '\n') {
                return read_count; // Found newline
            }
        }
        
        // Move to next piece
        piece_index++;
        offset = 0;
    }
    
    return read_count;
}

void Document::InsertAt(size_t ptd_index, char c) {
    InsertAt(ptd_index, std::string(1, c));
}

void Document::InsertAt(size_t ptd_index, const std::string &str) {
    if (str.empty()) return;
    
    WasEdited = true;
    
    // Handle insertion at end of document
    size_t doc_length = GetLength();
    if (ptd_index == doc_length) {
        // Append to add buffer and create new piece
        size_t start_pos = Buffers.add.length();
        Buffers.add.append(str);
        pieces.push_back(Document::Piece(BufferType::Add, start_pos, str.length()));
        return;
    }
    
    // Handle insertion at beginning or middle
    if (ptd_index > doc_length) return; // Out of range
    
    size_t piece_index;
    size_t offset;
    
    if (!FindPieceAt(ptd_index, piece_index, offset)) {
        return;
    }
    
    // Add the new string to the add buffer
    size_t add_start = Buffers.add.length();
    Buffers.add.append(str);
    
    // Split the piece if insertion is in the middle
    if (offset > 0 && offset < pieces[piece_index].Length) {
        Piece& current_piece = pieces[piece_index];
        Piece second_half(current_piece.Type, current_piece.Start + offset, current_piece.Length - offset);
        
        current_piece.Length = offset;
        
        // Insert new pieces: original part | new string | remaining original
        pieces.insert(pieces.begin() + piece_index + 1, Document::Piece(BufferType::Add, add_start, str.length()));
        pieces.insert(pieces.begin() + piece_index + 2, second_half);
    } else if (offset == 0) {
        // Insert at the beginning of this piece
        pieces.insert(pieces.begin() + piece_index, Document::Piece(BufferType::Add, add_start, str.length()));
    } else {
        // Insertion at end of piece (should not happen with FindPieceAt, but handle it)
        pieces.insert(pieces.begin() + piece_index + 1, Document::Piece(BufferType::Add, add_start, str.length()));
    }
}

size_t Document::DeleteRange(size_t ptd_index, size_t length) {
    if (length == 0) return 0;
    
    WasEdited = true;
    
    size_t doc_length = GetLength();
    
    // Clamp to valid range
    if (ptd_index >= doc_length) return 0;
    if (ptd_index + length > doc_length) {
        length = doc_length - ptd_index;
    }
    
    size_t deleted = 0;
    
    while (deleted < length && !pieces.empty()) {
        size_t piece_index;
        size_t offset;
        
        if (!FindPieceAt(ptd_index, piece_index, offset)) {
            break;
        }
        
        Piece& piece = pieces[piece_index];
        size_t to_delete = (length - deleted < piece.Length - offset) ? (length - deleted) : (piece.Length - offset);
        
        if (offset == 0 && to_delete == piece.Length) {
            // Delete entire piece
            pieces.erase(pieces.begin() + piece_index);
            deleted += to_delete;
        } else if (offset == 0) {
            // Delete from beginning of piece
            piece.Start += to_delete;
            piece.Length -= to_delete;
            deleted += to_delete;
        } else if (offset + to_delete == piece.Length) {
            // Delete from middle to end of piece
            piece.Length = offset;
            deleted += to_delete;
        } else {
            // Delete from middle - need to split
            Piece second_half(piece.Type, piece.Start + offset + to_delete, piece.Length - offset - to_delete);
            piece.Length = offset;
            pieces.insert(pieces.begin() + piece_index + 1, second_half);
            deleted += to_delete;
        }
    }
    
    return deleted;
}

void Document::ReplaceRange(size_t ptd_index, size_t length, const std::string &str) {
    DeleteRange(ptd_index, length);
    InsertAt(ptd_index, str);
}