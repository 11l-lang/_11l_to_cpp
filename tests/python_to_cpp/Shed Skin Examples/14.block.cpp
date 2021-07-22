#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

struct CodeBlock1
{
    CodeBlock1()
    {
        uR"(
This is a BLOCK compression algorithm
that uses the Huffman algorithm.

This simple block compressor assumes that the source file
is an exact multiple of the block length.
The encoding does not itself delimit the size of the file, so
the decoder needs to knows where the end of the compressed
file is. Thus we must either ensure the decoder knows
the original file's length, or we have to use a special
end-of-file character. A superior compressor would first
encode the source file's length at the head of the compressed
file; then the decoder would be able to stop at the right
point and correct any truncation errors. I'll fix this
in block2.py.

The package contains the following functions:

 findprobs(f=0.01,N=6):
    Find probabilities of all the events
    000000
    000001
     ...
    111111
    <-N ->

 Bencode(string,symbols,N):
    Reads in a string of 0s and 1s, forms blocks, and encodes with Huffman.

 Bdecode(string,root,N):
    Decode a binary string into blocks, then return appropriate 0s and 1s

 compress_it( inputfile, outputfile ):
    Make Huffman code, and compress

 uncompress_it( inputfile, outputfile ):
    Make Huffman code, and uncompress

 There are also three test functions.
 If block.py is run from a terminal, it invokes compress_it (using stdin)
 or uncompress_it (using stdin), respectively if there are zero arguments
 or one argument.

)"_S;
    }
} code_block_1;

class InternalNode
{
public:
    String name;
    decltype(0) leaf = 0;
    Array<InternalNode> child;
    InternalNode()
    {
    }

    template <typename T1, typename T2> auto children(const T1 &child0, const T2 &child1)
    {
        leaf = 0;
        child.append(child0);
        child.append(child1);
    }
};

class node
{
public:
    double count;
    int index;
    String name;
    InternalNode internalnode;
    decltype(u""_S) word = u""_S;
    decltype(0) isinternal = 0;
    template <typename T1, typename T2, typename T3 = decltype(u""_S)> node(const T1 &count, const T2 &index, const T3 &name = u""_S) :
        index(index),
        name(name)
    {
        this->count = to_float(count);
        if (this->name == u"")
            this->name = u"_"_S & String(index);
    }
    template <typename T1> auto operator<(const T1 &other) const
    {
        return count < other.count;
    }
    auto report()
    {
        if ((index == 1))
            print(u"#Symbol\tCount\tCodeword"_S);
        print(u"#.\t(#.6)\t#."_S.format(name, count, word));
    }
    template <typename T1> auto associate(const T1 &internalnode)
    {
        this->internalnode = internalnode;
        this->internalnode.leaf = 1;
        this->internalnode.name = name;
    }
};

template <typename T1, typename T2> auto find_idx(const T1 &seq, const T2 &index)
{
    for (auto &&item : seq)
        if (item.index == index)
            return item;
}

template <typename T1, typename T2> auto find_name(const T1 &seq, const T2 &name)
{
    for (auto &&item : seq)
        if (item.name == name)
            return item;
    return node(0, -1);
}

InternalNode iterate(Array<node> &c)
{
    u"\n\
    Run the Huffman algorithm on the list of \"nodes\" c.\n\
    The list of nodes c is destroyed as we go, then recreated.\n\
    Codewords 'co.word' are assigned to each node during the recreation of the list.\n\
    The order of the recreated list may well be different.\n\
    Use the list c for encoding.\n\
\n\
    The root of a new tree of \"internalnodes\" is returned.\n\
    This root should be used when decoding.\n\
\n\
    >>> c = [ node(0.5,1,'a'),  \
              node(0.25,2,'b'), \
              node(0.125,3,'c'),\
              node(0.125,4,'d') ]   # my doctest query has been resolved\n\
    >>> root = iterate(c)           # \"iterate(c)\" returns a node, not nothing, and doctest cares!\n\
    >>> reportcode(c)               # doctest: +NORMALIZE_WHITESPACE, +ELLIPSIS\n\
    #Symbol   Count	Codeword\n\
    a         (0.5)	1\n\
    b         (0.25)	01\n\
    c         (0.12)	000\n\
    d         (0.12)	001\n\
    "_S;
    InternalNode root;
    if ((c.len() > 1)) {
        c.sort();
        auto deletednode = _get<0>(c);
        auto second = _get<1>(c).index;
        _get<1>(c).count += _get<0>(c).count;
        c.pop(0);

        root = iterate(c);

        auto co = find_idx(c, second);
        deletednode.word = co.word & u"0"_S;
        c.append(deletednode);
        co.word &= u"1"_S;
        co.count -= deletednode.count;

        auto newnode0 = InternalNode();
        auto newnode1 = InternalNode();
        auto treenode = co.internalnode;
        treenode.children(newnode0, newnode1);
        deletednode.associate(newnode0);
        co.associate(newnode1);
    }
    else {
        _get<0>(c).word = u""_S;
        root = InternalNode();
        _get<0>(c).associate(root);
    }
    return root;
}

template <typename T1, typename T2> auto encode(const T1 &sourcelist, const T2 &code)
{
    uR"(
    Takes a list of source symbols. Returns a binary string.
    )"_S;
    auto answer = u""_S;
    for (auto &&s : sourcelist) {
        auto co = find_name(code, s);
        if (co.index == -1)
            print(u"Warning: symbol "_S & s & u" has no encoding!"_S);
        else
            answer = answer & co.word;
    }
    return answer;
}

template <typename T1, typename T2> auto decode(const T1 &string, const T2 &root)
{
    uR"(
    Decodes a binary string using the Huffman tree accessed via root
    )"_S;
    Array<String> answer;
    auto clist = create_array(string);
    auto currentnode = root;
    for (auto &&c : clist) {
        if ((c == u'\n'))
            continue;
        assert((c == u'0') || (c == u'1'));
        currentnode = currentnode.child[to_int(c)];
        if (currentnode.leaf != 0) {
            answer.append(String(currentnode.name));
            currentnode = root;
        }
    }
    return answer;
}

template <typename T1> auto makenodes(const T1 &probs)
{
    uR"(
    Creates a list of nodes ready for the Huffman algorithm.
    Each node will receive a codeword when Huffman algorithm "iterate" runs.

    probs should be a list of pairs('<symbol>', <value>).

    >>> probs=[('a',0.5), ('b',0.25), ('c',0.125), ('d',0.125)]
    >>> symbols = makenodes(probs)
    >>> root = iterate(symbols)
    >>> zipped = encode(['a','a','b','a','c','b','c','d'], symbols)
    >>> print zipped
    1101100001000001
    >>> print decode( zipped, root )
    ['a', 'a', 'b', 'a', 'c', 'b', 'c', 'd']

    See also the file Example.py for a python program that uses this package.
    )"_S;
    auto m = 0;
    Array<node> c;
    for (auto &&p : probs) {
        m++;
        c.append(node(_get<1>(p), m, _get<0>(p)));
    }
    return c;
}

template <typename T1, typename T2> auto dec_to_bin(const T1 &n, const T2 &digits)
{
    uR"( n is the number to convert to binary;  digits is the number of bits you want
    Always prints full number of digits
    >>> print dec_to_bin( 17 , 9)
    000010001
    >>> print dec_to_bin( 17 , 5)
    10001

    Will behead the standard binary number if requested
    >>> print dec_to_bin( 17 , 4)
    0001
    )"_S;
    if ((n < 0))
        _stderr.write(u"warning, negative n not expected\n"_S);
    auto i = digits - 1;
    auto ans = u""_S;
    while (i >= 0) {
        auto b = (((1 << i) & n) > 0);
        i--;
        ans = ans & String(to_int(b));
    }
    return ans;
}

auto verbose = 0;

template <typename T1> auto weight(const T1 &string)
{
    uR"(
    ## returns number of 0s and number of 1s in the string
    >>> print weight("00011")
    (3, 2)
    )"_S;
    auto w0 = 0;
    auto w1 = 0;
    for (auto &&c : create_array(string))
        if ((c == u'0'))
            w0++;
        else if ((c == u'1'))
            w1++;
    return make_tuple(w0, w1);
}

template <typename T1 = decltype(0.01), typename T2 = decltype(6)> auto findprobs(const T1 &f = 0.01, const T2 &nn = 6)
{
    uR"( Find probabilities of all the events
    000000
    000001
     ...
    111111
    <-nn ->
    >>> print findprobs(0.1,3)              # doctest:+ELLIPSIS
    [('000', 0.7...),..., ('111', 0.001...)]
    )"_S;
    Array<Tuple<String, double>> answer;
    for (auto n : range_el(0, to_int(pow(2, nn)))) {
        auto s = dec_to_bin(n, nn);
        auto [w0, w1] = weight(s);
        if (::verbose && 0)
            print(s & u" "_S & w0 & u" "_S & w1);
        answer.append(make_tuple(s, pow(f, w1) * pow((1 - f), w0)));
    }
    assert(answer.len() == pow(2, nn));
    return answer;
}

template <typename T1, typename T2, typename T3> auto Bencode(const T1 &string, const T2 &symbols, const T3 &n)
{
    uR"(
    Reads in a string of 0s and 1s.
    Creates a list of blocks of size n.
    Sends this list to the general-purpose Huffman encoder
    defined by the nodes in the list "symbols".
    )"_S;
    Array<String> blocks;
    auto chars = create_array(string);

    auto s = u""_S;
    for (auto &&c : chars) {
        s = s & c;
        if ((s.len() >= n)) {
            blocks.append(s);
            s = u""_S;
        }
    }
    if ((s.len() > 0)) {
        print(u"warning, padding last block with 0s"_S);
        while ((s.len() < n))
            s = s & u"0"_S;
        blocks.append(s);
    }

    if (::verbose) {
        print(u"blocks to be encoded:"_S);
        print(blocks);
    }
    auto zipped = encode(blocks, symbols);
    return zipped;
}

template <typename T1, typename T2, typename T3> auto Bdecode(const T1 &string, const T2 &root, const T3 &n)
{
    uR"(
    Decode a binary string into blocks.
    )"_S;
    auto answer = decode(string, root);
    if (::verbose) {
        print(u"blocks from decoder:"_S);
        print(answer);
    }
    auto output = answer.join(u""_S);
    return output;
}

auto easytest()
{
    uR"(
    Tests block code with N=3, f=0.01 on a tiny example.
    >>> easytest()                 # doctest:+NORMALIZE_WHITESPACE
    #Symbol	Count	        Codeword
    000	        (0.97)	        1
    001	        (0.0098)	001
    010	        (0.0098)	010
    011	        (9.9e-05)	00001
    100	        (0.0098)	011
    101	        (9.9e-05)	00010
    110	        (9.9e-05)	00011
    111	        (1e-06) 	00000
    zipped  = 1001010000010110111
    decoded = ['000', '001', '010', '011', '100', '100', '000']
    OK!
    )"_S;
    auto n = 3;
    auto f = 0.01;
    auto probs = findprobs(f, n);
    auto symbols = makenodes(probs);
    auto root = iterate(symbols);

    symbols = sorted(symbols, [](const auto &x){return x.index;});
    for (auto &&co : symbols)
        co.report();

    auto source = create_array({u"000"_S, u"001"_S, u"010"_S, u"011"_S, u"100"_S, u"100"_S, u"000"_S});
    auto zipped = encode(source, symbols);
    print(u"zipped  = "_S & zipped);
    auto answer = decode(zipped, root);
    print(u"decoded = "_S, u""_S);
    print(answer);
    if ((source != answer))
        print(u"ERROR"_S);
    else
        print(u"OK!"_S);
}

auto f = 0.01;
auto n = 12;

struct CodeBlock2
{
    CodeBlock2()
    {
        f = 0.01;
        n = 5;
        f = 0.01;
        n = 10;
    }
} code_block_2;

template <typename T1, typename T2> auto compress_it(const T1 &inputfile, const T2 &outputfile)
{
    uR"(
    Make Huffman code for blocks, and
    Compress from file (possibly stdin).
    )"_S;
    auto probs = findprobs(::f, ::n);
    auto symbols = makenodes(probs);
    auto root = iterate(symbols);

    auto string = inputfile.read();
    outputfile.write(Bencode(string, symbols, ::n));
}

template <typename T1, typename T2> auto uncompress_it(const T1 &inputfile, const T2 &outputfile)
{
    uR"(
    Make Huffman code for blocks, and
    UNCompress from file (possibly stdin).
    )"_S;
    auto probs = findprobs(::f, ::n);
    auto symbols = makenodes(probs);
    auto root = iterate(symbols);

    auto string = inputfile.read();
    outputfile.write(Bdecode(string, root, ::n));
}

auto hardertest()
{
    print(u"Reading the BentCoinFile"_S);
    auto inputfile = File(u"testdata/BentCoinFile"_S, u"r"_S);
    auto outputfile = File(u"tmp.zip"_S, u"w"_S);
    print(u"Compressing to tmp.zip"_S);
    compress_it(inputfile, outputfile);
    outputfile.close();
    inputfile.close();

    inputfile = File(u"tmp.zip"_S, u"r"_S);
    outputfile = File(u"tmp2"_S, u"w"_S);
    print(u"Uncompressing to tmp2"_S);
    uncompress_it(inputfile, outputfile);
    outputfile.close();
    inputfile.close();

    print(u"Checking for differences..."_S);
    print((File(u"testdata/BentCoinFile"_S).read() == File(u"tmp2"_S).read()));
}

auto test()
{
    easytest();
    hardertest();
}

int main()
{
    test();
}
