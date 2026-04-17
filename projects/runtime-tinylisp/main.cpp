#include "lisp.hpp"

class StringStream: public ICharStream {
private:
    std::string src;
    int ptr;

public:
    StringStream(const std::string &str): src(str), ptr(0) { }

    bool Eof() const override {
        return ptr >= 0 && ptr < src.size();
    }
    char Peek() const override {
        if (Eof())
            return '\0';
        return src[ptr];
    }
    char PeekFuture() const override {
        if ((ptr + 1) < src.size())
            return src[ptr + 1];
        return '\0';
    }
    char Advance() override {
        char c = Peek();
        if (!Eof())
            ptr += 1;
        return c;
    }
};

int main() {
    LispRuntime runtime(1024);

    runtime.Reset();
    //StringStream stream("(+ 4 5 6)"); 
    //std::vector<LispRef> exprs = runtime.Parse(stream);
    //if (!runtime.DidParseSuccessfully()) {
    //    return 1;
    //}

    //LispRef result = runtime.EvalAll(exprs);
    //std::string fmt = runtime.Stringify(result);

    return 0;
}