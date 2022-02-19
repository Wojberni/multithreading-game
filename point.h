#ifndef __POINT__
#define __POINT__

struct Point
{
    int x;
    int y;
    Point(int _x=0, int _y=0): x(_x), y(_y) {};
    Point& operator+=(const Point& delta)
    {
        x+=delta.x;
        y+=delta.y;
        return *this;
    }
    Point operator+(const Point& delta) const
    {
        return Point(x + delta.x, y + delta.y);
    }
    bool operator==(const Point& second)
    {
        if(this->x == second.x && this->y == second.y)
            return true;
        return false;
    }

};

struct MovingObject
{
    Point left;
    Point right;
    MovingObject(int _x=0, int _y=0): left(_x, _y), right(_x + 1, _y) {};

    MovingObject& operator+=(const Point& delta)
    {
        left.x += delta.x;
        left.y += delta.y;
        right.x += delta.x;
        right.y += delta.y;
        return *this;
    }

    bool operator==(const MovingObject& second)
    {
        if(this->left.x == second.left.x && this->left.y == second.left.y
        && this->right.x == second.right.x && this->right.y == second.right.y)
            return true;
        return false;
    }
};

struct Collectible{
    Point point;
    int value;
    Collectible(int _x, int _y, int _value): point(_x, _y), value(_value) {};
};

#endif