#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

struct CodeBlock1
{
    CodeBlock1()
    {
        uR"(
Amaze - A completely object-oriented Pythonic maze generator/solver.
This can generate random mazes and solve them. It should be
able to solve any kind of maze and inform you in case a maze is
unsolveable.

This uses a very simple representation of a mze. A maze is
represented as an mxn matrix with each point value being either
0 or 1. Points with value 0 represent paths and those with
value 1 represent blocks. The problem is to find a path from
point A to point B in the matrix.

The matrix is represented internally as a list of lists.

Have fun ☺
http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/496884
)"_S;
    }
} code_block_1;

class MazeReaderException
{
public:
    template <typename T1> MazeReaderException(const T1 &message)
    {
    }
};

auto STDIN = 0;
auto FILE_ = 1;
auto SOCKET = 2;

auto PATH = -1;
auto START = -2;
auto EXIT = -3;

auto null_point = make_tuple(1 << 30, 1 << 30);

class MazeReader
{
public:
    Array<Array<int>> maze_rows;

    MazeReader()
    {
    }

    auto readStdin()
    {
        print(u"Enter a maze"_S);
        print(u"You can enter a maze row by row"_S);
        print();

        auto data = input(u"Enter the dimension of the maze as Width X Height: "_S);
        auto [w1, h1] = bind_array<2>(data.split_py());
        auto [w, h] = make_tuple(to_int(w1), to_int(h1));

        for (auto x : range_el(0, h)) {
            auto row = u""_S;
            while (row == u"")
                row = input(u"Enter row number #.: "_S.format(x + 1));
            auto rowsplit = row.split_py().map([](const auto &y){return to_int(y);});
            if (rowsplit.len() != w)
                throw MazeReaderException(u"invalid size of maze row"_S);
            maze_rows.append(rowsplit);
        }
    }

    auto readFile()
    {
        auto fname = u"testdata/maze.txt"_S;
        try
        {
            auto f = File(fname);
            auto lines = f.read_lines(true);
            f.close();
            auto workaround_for_MSVC_2017 = [](const String &line)
            {
                return line.trim(make_tuple(u" "_S, u"\t"_S, u"\r"_S, u"\n"_S)) != u"";
            };
            lines = lines.filter([&workaround_for_MSVC_2017](const auto &line){return workaround_for_MSVC_2017(line);});
            auto w = _get<0>(lines).split_py().len();
            for (auto &&line : lines) {
                auto row = line.split_py().map([](const auto &y){return to_int(y);});
                if (row.len() != w)
                    throw MazeReaderException(u"Invalid maze file - error in maze dimensions"_S);
                else
                    maze_rows.append(row);
            }
        }
        catch (...)
        {
            throw MazeReaderException(u"read error"_S);
        }
    }

    auto getData()
    {
        return maze_rows;
    }

    template <typename T1 = decltype(STDIN)> auto readMaze(const T1 &source = STDIN)
    {
        if (source == ::STDIN)
            readStdin();
        else if (source == ::FILE_)
            readFile();

        return getData();
    }
};

class MazeError
{
public:
    template <typename T1> MazeError(const T1 &message)
    {
    }
};

class Maze
{
public:
    Array<Array<int>> _rows;
    int _height;
    int _width;

    template <typename T1> Maze(const T1 &rows) :
        _rows(rows)
    {
        __validate();
        __normalize();
    }

    operator String() const
    {
        auto s = u"\n"_S;
        for (auto &&row : _rows) {
            for (auto &&item : row) {
                String sitem;
                if (item == ::PATH)
                    sitem = u'*'_C;
                else if (item == ::START)
                    sitem = u'S'_C;
                else if (item == ::EXIT)
                    sitem = u'E'_C;
                else
                    sitem = String(item);

                s = s & u"  "_S & sitem & u"   "_S;
            }
            s = s & u"\n\n"_S;
        }

        return s;
    }

    auto __validate()
    {
        auto width = _get<0>(_rows).len();
        auto widths = _rows.map([](const auto &row){return row.len();});
        if (widths.count(width) != widths.len())
            throw MazeError(u"Invalid maze!"_S);

        _height = _rows.len();
        _width = width;
    }

    auto __normalize()
    {
        for (auto x : range_el(0, _rows.len())) {
            auto row = _rows[x];
            row = row.map([](const auto &y){return min(to_int(y), 1);});
            _rows.set(x, row);
        }
    }

    template <typename T1> auto validatePoint(const T1 &pt) const
    {
        auto [x, y] = pt;
        auto w = _width;
        auto h = _height;

        if (x > w - 1 || x < 0)
            throw MazeError(u"x co-ordinate out of range!"_S);

        if (y > h - 1 || y < 0)
            throw MazeError(u"y co-ordinate out of range!"_S);
    }

    template <typename T1, typename T2> auto getItem(const T1 &x, const T2 &y)
    {
        validatePoint(make_tuple(x, y));

        auto w = _width;
        auto h = _height;

        auto row = _rows[h - y - 1];
        return row[x];
    }

    template <typename T1, typename T2, typename T3> auto setItem(const T1 &x, const T2 &y, const T3 &value)
    {
        auto h = _height;

        validatePoint(make_tuple(x, y));
        auto row = _rows[h - y - 1];
        row.set(x, value);
    }

    template <typename T1> auto getNeighBours(const T1 &pt)
    {
        validatePoint(pt);

        auto [x, y] = pt;

        auto h = _height;
        auto w = _width;

        auto poss_nbors = make_tuple(make_tuple(x - 1, y), make_tuple(x - 1, y + 1), make_tuple(x, y + 1), make_tuple(x + 1, y + 1), make_tuple(x + 1, y), make_tuple(x + 1, y - 1), make_tuple(x, y - 1), make_tuple(x - 1, y - 1));

        Array<ivec2> nbors;
        for (auto &&[xx, yy] : poss_nbors)
            if ((xx >= 0 && xx <= w - 1) && (yy >= 0 && yy <= h - 1))
                nbors.append(make_tuple(xx, yy));

        return nbors;
    }

    template <typename T1> auto getExitPoints(const T1 &pt)
    {
        Array<ivec2> exits;
        for (auto &&[xx, yy] : getNeighBours(pt))
            if (getItem(xx, yy) == 0)
                exits.append(make_tuple(xx, yy));

        return exits;
    }

    template <typename T1, typename T2> auto calcDistance(const T1 &pt1, const T2 &pt2)
    {
        validatePoint(pt1);
        validatePoint(pt2);

        auto [x1, _y1_] = pt1;
        auto [x2, y2] = pt2;

        return pow((pow((x1 - x2), 2) + pow((_y1_ - y2), 2)), 0.5);
    }
};

class MazeFactory
{
public:
    template <typename T1 = decltype(STDIN)> auto makeMaze(const T1 &source = STDIN)
    {
        auto reader = MazeReader();
        return Maze(reader.readMaze(source));
    }
};

class MazeSolver
{
public:
    Maze maze;
    decltype(make_tuple(0, 0)) _start = make_tuple(0, 0);
    decltype(make_tuple(0, 0)) _end = make_tuple(0, 0);
    decltype(make_tuple(0, 0)) _current = make_tuple(0, 0);
    decltype(0) _steps = 0;
    Array<ivec2> _path;
    decltype(false) _tryalternate = false;
    decltype(false) _trynextbest = false;
    decltype(make_tuple(0, 0)) _disputed = make_tuple(0, 0);
    decltype(0) _loops = 0;
    decltype(false) _retrace = false;
    decltype(0) _numretraces = 0;

    template <typename T1> MazeSolver(const T1 &maze) :
        maze(maze)
    {
    }

    template <typename T1> auto setStartPoint(const T1 &pt)
    {
        maze.validatePoint(pt);
        _start = pt;
    }

    template <typename T1> auto setEndPoint(const T1 &pt)
    {
        maze.validatePoint(pt);
        _end = pt;
    }

    auto boundaryCheck()
    {
        auto exits1 = maze.getExitPoints(_start);
        auto exits2 = maze.getExitPoints(_end);

        if (exits1.empty() || exits2.empty())
            return false;

        return true;
    }

    template <typename T1> auto setCurrentPoint(const T1 &point)
    {
        _current = point;
        _path.append(point);
    }

    auto isSolved()
    {
        return (_current == _end);
    }

    auto retracePath()
    {
        print(u"Retracing..."_S);
        _retrace = true;

        auto path2 = copy(_path);
        path2.reverse();

        auto idx = path2.index(_start);
        _path.extend(_path[range_el(_path.len() - 2, idx).step(-1)]);
        _numretraces++;
    }

    auto endlessLoop()
    {
        if (_loops > 100) {
            print(u"Seems to be hitting an endless loop."_S);
            return true;
        }
        else if (_numretraces > 8) {
            print(u"Seem to be retracing loop."_S);
            return true;
        }

        return false;
    }

    auto getNextPoint()
    {
        auto points = maze.getExitPoints(_current);

        auto point = getBestPoint(points);

        while (checkClosedLoop(point)) {

            if (endlessLoop()) {
                print(_loops);
                point = ::null_point;
                break;
            }

            auto point2 = point;
            if (point == _start && _path.len() > 2) {
                _tryalternate = true;
                break;
            }
            else {
                point = getNextClosestPointNotInPath(points, point2);
                if (point == ::null_point) {
                    retracePath();
                    _tryalternate = true;
                    point = _start;
                    break;
                }
            }
        }

        return point;
    }

    template <typename T1> auto checkClosedLoop(const T1 &point)
    {
        auto l = create_array(range_el(0, _path.len() - 1).step(2));
        l.reverse();

        for (auto &&x : l)
            if (_path[x] == point) {
                _loops++;
                return true;
            }

        return false;
    }

    template <typename T1> auto getBestPoint(const T1 &points)
    {
        auto point = getClosestPoint(points);
        auto point2 = point;
        auto altpoint = point;

        if (in(point2, _path)) {
            point = getNextClosestPointNotInPath(points, point2);
            if (point == ::null_point)
                point = point2;
        }

        if (_tryalternate) {
            point = getAlternatePoint(points, altpoint);
            print(u"Trying alternate... "_S, u""_S);
            print(_current, u" "_S);
            print(point);
        }

        _trynextbest = false;
        _tryalternate = false;
        _retrace = false;

        return point;
    }

    template <typename T1> auto sortPoints(const T1 &points)
    {
        auto distances = points.map([this](const auto &point){return maze.calcDistance(point, _end);});
        auto distances2 = copy(distances);

        distances.sort();

        auto points2 = create_array({make_tuple(0, 0)}) * points.len();
        auto count = 0;

        for (auto &&dist : distances) {
            auto idx = distances2.index(dist);
            auto point = points[idx];

            while (in(point, points2)) {
                idx = distances2.index(dist, idx + 1);
                point = points[idx];
            }

            points2.set(count, point);
            count++;
        }

        return points2;
    }

    template <typename T1> auto getClosestPoint(const T1 &points)
    {
        auto points2 = sortPoints(points);

        auto closest = _get<0>(points2);
        return closest;
    }

    template <typename T1, typename T2> auto getAlternatePoint(const T1 &points, const T2 &point)
    {
        auto points2 = copy(points);
        print(points2, u" "_S);
        print(point);

        points2.remove(point);
        if (!points2.empty())
            return randomns::choice(points2);

        return ::null_point;
    }

    template <typename T1, typename T2> auto getNextClosestPoint(const T1 &points, const T2 &point)
    {
        auto points2 = sortPoints(points);
        auto idx = points2.index(point);

        try
        {
            return points2[idx + 1];
        }
        catch (...)
        {
            return ::null_point;
        }
    }

    template <typename T1, typename T2> auto getNextClosestPointNotInPath(const T1 &points, const T2 &point)
    {

        auto point2 = getNextClosestPoint(points, point);
        while (in(point2, _path))
            point2 = getNextClosestPoint(points, point2);

        return point2;
    }

    auto printResult()
    {
        u" Print the maze showing the path "_S;

        for (auto &&[x, y] : _path)
            maze.setItem(x, y, ::PATH);

        maze.setItem(_get<0>(_start), _get<1>(_start), ::START);
        maze.setItem(_get<0>(_end), _get<1>(_end), ::EXIT);
    }

    auto solve()
    {

        if (_start == _end) {
            print(u"Start/end points are the same. Trivial maze."_S);
            print(create_array({_start, _end}));
            return;
        }

        if (!boundaryCheck()) {
            print(u"Either start/end point are unreachable. Maze cannot be solved."_S);
            return;
        }

        setCurrentPoint(_start);

        auto unsolvable = false;

        while (!isSolved()) {
            _steps++;
            auto pt = getNextPoint();

            if (pt != ::null_point)
                setCurrentPoint(pt);
            else {
                print(u"Dead-lock - maze unsolvable"_S);
                unsolvable = true;
                break;
            }
        }

        if (!unsolvable) {
        }

        else {
            print(u"Path till deadlock is "_S, u""_S);
            print(_path);
        }

        printResult();
    }
};

class MazeGame
{
public:
    decltype(make_tuple(0, 0)) _start = make_tuple(0, 0);
    decltype(make_tuple(0, 0)) _end = make_tuple(0, 0);
    MazeGame()
    {
    }

    virtual Maze createMaze() = 0;

    virtual void getStartEndPoints(const Maze &maze) = 0;

    auto runGame()
    {
        auto maze = createMaze();

        getStartEndPoints(maze);

        auto solver = MazeSolver(maze);

        solver.setStartPoint(_start);
        solver.setEndPoint(_end);
        solver.solve();
    }
};

class FilebasedMazeGame : public MazeGame
{
public:

    virtual Maze createMaze() override
    {
        auto f = MazeFactory();
        auto m = f.makeMaze(::FILE_);
        print(m);
        return m;
    }

    virtual void getStartEndPoints(const Maze &maze) override
    {

        while (true) {
            try
            {
                auto pt1 = u"0 4"_S;
                auto [x, y] = bind_array<2>(pt1.split_py());
                _start = make_tuple(to_int(x), to_int(y));
                maze.validatePoint(_start);
                break;
            }
            catch (...)
            {
            }
        }

        while (true) {
            try
            {
                auto pt2 = u"5 4"_S;
                auto [x, y] = bind_array<2>(pt2.split_py());
                _end = make_tuple(to_int(x), to_int(y));
                maze.validatePoint(_end);
                break;
            }
            catch (...)
            {
            }
        }
    }
};

int main()
{
    auto game = std::make_unique<FilebasedMazeGame>();
    for (int x = 0; x < 1; x++)
        game->runGame();
}
