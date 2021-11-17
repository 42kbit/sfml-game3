#include "SFML/Graphics.hpp"
#include <sstream>
namespace game
{
    const uint8_t width = 3, height = 3;
    const uint8_t outlineThickness = 2;
    struct CellType
    {
        sf::Texture texture;
    };

    CellType cellNone;
    CellType cellNought;
    CellType cellCross;

    struct Cell
    {
        CellType* type;
    };

    struct World
    {
        
        CellType* lastPlaced;
        Cell** grid;
    };

    World createWorld()
    {
        World world;
        world.grid = new Cell*[width];
        for(uint32_t i = 0; i < width; i++)
        {
            world.grid[i] = new Cell[height];
        }

        for(uint32_t i = 0; i < width; i++){
            for(uint32_t j = 0; j < height; j++)
            {
                world.grid[i][j].type = &cellNone; 
            }
        }
        world.lastPlaced = &cellCross;

        return world;
    }

    void deleteWorld(World* world)
    {
        for(uint32_t i = 0; i < width; i++)
        {
            delete[] world->grid[i];
        }
        delete[] world->grid;
    }

    void drawWorld(sf::RenderWindow* window, World* world)
    {
        for(uint32_t i = 0; i < width; i++)
        {
            for(uint32_t j = 0; j < height; j++)
            {
                sf::RectangleShape rect;
                sf::Texture texture;
                const sf::Vector2u& windowSize = window->getSize();
                rect.setSize(sf::Vector2f(windowSize.x / width, windowSize.y / height));
                rect.setPosition(sf::Vector2f(i * rect.getSize().x + outlineThickness/2, 
                                              j * rect.getSize().y + outlineThickness/2 ));
                rect.setOutlineThickness(outlineThickness);
                rect.setOutlineColor(sf::Color::Black);
                rect.setTexture(&world->grid[i][j].type->texture);

                window->draw(rect);
            }
        }
    }

    void initTextures(char* noughtPath, char* crossPath)
    {
        cellNought.texture.loadFromFile(noughtPath);
        cellCross.texture.loadFromFile(crossPath);
    }

    void swapCells(World* world)
    {
        if(world->lastPlaced == &cellNought)
            world->lastPlaced = &cellCross;
        else
            world->lastPlaced = &cellNought;
    }

    Cell* getCellAt(World* world, uint32_t x, uint32_t y)
    {
        return &world->grid[x][y];
    }

    bool isCellInWorld(World* world, uint32_t x, uint32_t y)
    {
        return (x >= 0) && (x < width) && (y >= 0) && (y < height);
    }

    CellType* checkWin(World* world, uint32_t x, uint32_t y)
    {
        CellType* type = getCellAt(world, x, y)->type;
        uint32_t max_count = 1;
        for(int32_t i = -1; i < 2; i++)
        {
            for(int32_t j = -1; j < 2; j++)
            {
                if(isCellInWorld(world, x + i, y + j) && (i != 0 || j != 0))
                {
                    uint32_t count = 1;
                    int32_t dx=i, dy=j;
                    while(isCellInWorld(world, x+dx, y+dy) && getCellAt(world, x+dx, y+dy)->type == type){
                        count++;
                        dx+=i;
                        dy+=j;
                    }
                    dx=-i, dy=-j;
                    while(isCellInWorld(world, x+dx, y+dy) && getCellAt(world, x+dx, y+dy)->type == type){
                        count++;
                        dx+=-i;
                        dy+=-j;
                        
                    }
                    if(count > max_count)
                        max_count = count;
                }
            }
        }
        if(max_count >= 3)
            return type;
        return &cellNone;
    }

    

    CellType* setWorldCell(World* world, uint32_t x, uint32_t y)
    {
        if(world->grid[x][y].type == &cellNone){
            world->grid[x][y].type = world->lastPlaced;
            CellType* winner = checkWin(world, x, y);
            swapCells(world);
            return winner;
        }
        return &cellNone;
    }

    void cleanMap(World* world)
    {
        for(uint32_t i = 0; i < width; i++)
            for(uint32_t j = 0; j < height; j++)
                world->grid[i][j].type = &cellNone;        
    }

    void setWinner(CellType* winner, sf::RenderWindow* window)
    {
        std::stringstream ss;
        ss << "The last winner is: " << ((winner == &cellNought)? "NOUGHT" : "CROSS") << "!";
        window->setTitle(ss.str());
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(512, 512), "Tic Tac Toe", sf::Style::Default);
    window.setFramerateLimit(30);
    sf::Event ev;

    game::World world = game::createWorld();
    game::initTextures("res/textures/nought.png", "res/textures/cross.png");

    while(window.isOpen())
    {
        while(window.pollEvent(ev))
        {
            if(ev.type == sf::Event::Closed) 
                window.close();
            if(ev.type == sf::Event::MouseButtonPressed && 
               ev.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2i mPos = sf::Mouse::getPosition(window);
                sf::Vector2f worldIndex = sf::Vector2f(((float)mPos.x / (float)window.getSize().x) * game::width, 
                                                       ((float)mPos.y / (float)window.getSize().y) * game::height);
                game::CellType* winner = setWorldCell(&world, worldIndex.x, worldIndex.y);
                if(winner != &game::cellNone){
                    game::setWinner(winner, &window);
                    game::cleanMap(&world);
                }
            }
            
        }

        window.clear(sf::Color::White);
        game::drawWorld(&window, &world);
        window.display();
    }

    game::deleteWorld(&world);
}