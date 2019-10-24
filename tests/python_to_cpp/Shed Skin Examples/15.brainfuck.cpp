#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

template <typename T1> auto BF_interpreter(T1 prog)
{
    auto CELL = 255;
    prog = (prog.filter([](const auto &c){return in(c, u"><+-.,[]"_S);})).join(u""_S);
    auto len_prog = prog.len();
    auto tape = create_array({0});
    auto ip = 0;
    auto p = 0;
    auto level = 0;

    while (ip < len_prog) {
        auto x = prog[ip];
        ip++;
        if (x == u'+')
            tape.set(p, (tape[p] + 1) & CELL);
        else if (x == u'-')
            tape.set(p, (tape[p] - 1) & CELL);
        else if (x == u'>') {
            p++;
            if (tape.len() <= p)
                tape.append(0);
        }
        else if (x == u'<') {
            if (p)
                p--;
            else
                tape.insert(0, 0);
        }
        else if (x == u'.')
            _stdout.write(Char(tape[p]));
        else if (x == u',')
            tape.set(p, _stdin.read(1).code);
        else if (x == u'[') {
            if (!(tape[p])) {
                while (true) {
                    if (prog[ip] == u'[')
                        level++;
                    if (prog[ip] == u']') {
                        if (level)
                            level--;
                        else
                            break;
                    }
                    ip++;
                }
                ip++;
            }
        }
        else if (x == u']') {
            ip -= 2;
            while (true) {
                if (prog[ip] == u'[') {
                    if (level)
                        level--;
                    else
                        break;
                }
                if (prog[ip] == u']')
                    level++;
                ip--;
            }
        }
    }
}

int main()
{
    auto program = File(u"testdata/99bottles.bf"_S).read();
    BF_interpreter(program);
}
