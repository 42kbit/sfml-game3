#include "SFML/Graphics.hpp"
#include <sstream>
#include <iostream>
#include <list>

uint32_t width = 3, height = 3, num_to_win = 3;

bool path_set = false;
uint32_t cp_x = -1, cp_y = -1;
int32_t cpd_x = 0, cpd_y = 0;

const uint8_t outlineThickness = 1;
struct CellType
{
	sf::Texture texture;
};

struct CellType cellNone;
struct CellType cellNought;
struct CellType cellCross;

struct Cell
{
	struct CellType* type;
	int x, y;
};

struct World
{
	
	struct CellType* lastPlaced;
	struct Cell** grid;
	std::list <struct Cell*> gridFree;
};

World createWorld(uint32_t w, uint32_t h, uint32_t new_win)
{
	width = w;
	height = h;
	num_to_win = new_win;
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
			world.grid[i][j].x = i;
			world.grid[i][j].y = j;
			world.gridFree.push_back (&world.grid[i][j]);
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

struct CellType* getOpposite (struct CellType* p) {
	if (p == &cellCross)
		return &cellNought;
	else
		return &cellCross;
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

uint32_t countConsecutiveType (	struct World* world, struct CellType** types, int32_t ntypes,
				uint32_t x, uint32_t y, int32_t* pmdx, int32_t* pmdy)
{
	uint32_t max_count = 0;
	int32_t mdx = 0, mdy = 0;
	for(int32_t i = -1; i < 2; i++)
	{
		for(int32_t j = -1; j < 2; j++)
		{
			if(isCellInWorld(world, x + i, y + j) && (i != 0 || j != 0))
			{
				uint32_t count = 1;
				int32_t dx=i, dy=j;
				for (int k = 0; k < ntypes; k++) {
					while(isCellInWorld(world, x+dx, y+dy) && getCellAt(world, x+dx, y+dy)->type == types[k]){
						count++;
						dx+=i;
						dy+=j;
					}
				}
				dx=-i, dy=-j;
				for (int k = 0; k < ntypes; k++) {
					while(isCellInWorld(world, x+dx, y+dy) && getCellAt(world, x+dx, y+dy)->type == types[k]){
						count++;
						dx+=-i;
						dy+=-j;
						
					}
				}
				if(count > max_count) {
					max_count = count;
					mdx = i;
					mdy = j;
				}
			}
		}
	}
	if (pmdx)
		*pmdx = mdx;
	if (pmdy)
		*pmdy = mdy;
	return max_count;
}

CellType* checkWin(World* world, uint32_t x, uint32_t y)
{
	struct CellType* types [1] = {world->grid[x][y].type};
	uint32_t num = countConsecutiveType (world, types, 1, x, y, nullptr, nullptr);
	if(num >= num_to_win)
		return world->grid[x][y].type;
	return &cellNone;
}

bool setWorldCell(World* world, uint32_t x, uint32_t y)
{
	if(world->grid[x][y].type == &cellNone){
		world->grid[x][y].type = world->lastPlaced;
		world->gridFree.remove (&world->grid[x][y]);
		return true;
	}
	return false;
}

void cleanMap(World* world)
{
	for(uint32_t i = 0; i < width; i++)
		for(uint32_t j = 0; j < height; j++) {
			world->grid[i][j].type = &cellNone;		
			world->gridFree.push_back (&world->grid[i][j]);
		}
}

void setWinner(CellType* winner, sf::RenderWindow* window)
{
	std::stringstream ss;
	if (winner != &cellNone)
		ss << "The last winner is: " << ((winner == &cellNought)? "NOUGHT" : "CROSS") << "!";
	else
		ss << "DRAW" << "!";
	window->setTitle(ss.str());
}

bool isFilled(World* world)
{
	for(uint32_t i = 0; i < width; i++)
		for(uint32_t j = 0; j < height; j++)
			if(world->grid[i][j].type == &cellNone)
				return false;
	return true;

}

bool aiMoveRandom (struct World* world, uint32_t* px, uint32_t* py) {
	auto iter = world->gridFree.begin();
	if (iter == world->gridFree.end())
		return false;

	*px = (*iter)->x;
	*py = (*iter)->y;

	setWorldCell (world, (*iter)->x, (*iter)->y);
	return true;
}

bool aiMakeNew (struct World* world, uint32_t* px, uint32_t* py) {
	uint32_t max;
	for(uint32_t i = 0; i < width; i++) {
		for(uint32_t j = 0; j < height; j++) {
			if (world->grid[i][j].type != &cellNone) 
				continue;
			uint32_t newmax = 0;
			int32_t mdx, mdy;
			struct CellType* types[2] = {&cellNone, world->lastPlaced};
			newmax = countConsecutiveType(world, types, 2, i, j, &mdx, &mdy);
			if (newmax >= num_to_win) { /* pick that path */
				cp_x = i;
				cp_y = j;
				cpd_x = mdx;
				cpd_y = mdy;
				*px = cp_x;
				*py = cp_y;
				path_set = true;
				setWorldCell (world, cp_x, cp_y);
				return true;
			}
		}
	}
	return false;
}

bool aiContinue (struct World* world, uint32_t* px, uint32_t* py) {
	if (!path_set)
		return false;
	bool next_free_set = false;
	int32_t nf_x = 0, nf_y = 0;
	int32_t dx = cpd_x, dy = cpd_y;
	uint32_t count = 1;
	while(isCellInWorld(world, cp_x+dx, cp_y+dy) &&
		(getCellAt(world, cp_x+dx, cp_y+dy)->type == world->lastPlaced
		|| getCellAt(world, cp_x+dx, cp_y+dy)->type == &cellNone)){
		if (!next_free_set && getCellAt(world, cp_x+dx, cp_y+dy)->type == &cellNone) {
			nf_x = cp_x+dx;
			nf_y = cp_y+dy;
			next_free_set = true;
		}
		count++;
		dx+=cpd_x;
		dy+=cpd_y;
	}

	dx=-cpd_x, dy=-cpd_y;
	while(isCellInWorld(world, cp_x+dx, cp_y+dy) &&
		(getCellAt(world, cp_x+dx, cp_y+dy)->type == world->lastPlaced
		|| getCellAt(world, cp_x+dx, cp_y+dy)->type == &cellNone)){
		if (!next_free_set && getCellAt(world, cp_x+dx, cp_y+dy)->type == &cellNone) {
			nf_x = cp_x+dx;
			nf_y = cp_y+dy;
			next_free_set = true;
		}	
		count++;
		dx+=-cpd_x;
		dy+=-cpd_y;
	}
	if (count < num_to_win || !next_free_set) {
		path_set = false;
		return false;
	}
	setWorldCell (world, nf_x, nf_y);
	return true;
}

bool aiBlock (	struct World* world,
		uint32_t lx, uint32_t ly,
		uint32_t * px, uint32_t * py)
{
	uint32_t max_count = 0;
	int32_t mdx, mdy;
	uint32_t nf_x, nf_y;
	bool nf_set = false;
	bool next_found = false;
	struct CellType* type = getOpposite (world->lastPlaced);
	for(int32_t i = -1; i < 2; i++)
	{
		for(int32_t j = -1; j < 2; j++)
		{
			if(isCellInWorld(world, lx + i, ly + j) && (i != 0 || j != 0))
			{
				uint32_t count = 1;
				int32_t dx=i, dy=j;
				while(isCellInWorld(world, lx+dx, ly+dy)){
					if (getCellAt(world, lx+dx, ly+dy)->type == type) {
						count++;
					}
					dx+=i;
					dy+=j;
				}
				dx=-i, dy=-j;
				while(isCellInWorld(world, lx+dx, ly+dy)){
					if (getCellAt(world, lx+dx, ly+dy)->type == type) {
						count++;
					}
					dx+=-i;
					dy+=-j;
					
				}
				if(count > max_count) {
					max_count = count;
					mdx = i;
					mdy = j;
				}
			}
		}
	}
	int32_t dx=mdx, dy=mdy;
	while(isCellInWorld(world, lx+dx, ly+dy)){
		if (getCellAt(world, lx+dx, ly+dy)->type == &cellNone) {
			nf_x = lx+dx;
			nf_y = ly+dy;
			nf_set = true;
			goto place;
		}
		dx+=mdx;
		dy+=mdy;
	}
	dx=-mdy, dy=-mdx;
	while(isCellInWorld(world, lx+dx, ly+dy)){
		if (getCellAt(world, lx+dx, ly+dy)->type == &cellNone) {
			nf_x = lx+dx;
			nf_y = ly+dy;
			nf_set = true;
			goto place;
		}
		dx+=-mdx;
		dy+=-mdy;
		
	}
place:	;
	if (max_count >= (num_to_win - 1)) {
		if (!nf_set)
			return false;
		setWorldCell (world, nf_x, nf_y);
		return true;
	}
	return false;
}

/* Priority:
 * 1. Win
 * 2. Block
 * 3. Append existing if makes sense
 * 4. Make new if makes sense
 * 5. Place Random
 */
bool aiMove (
	struct World* world,
	uint32_t lx, uint32_t ly,
	uint32_t* px, uint32_t* py)
{
	if (aiBlock (world, lx, ly, px, py)) {
		return true;
	}
	if (aiContinue (world, px, py)) {
		return true;
	}
	if (aiMakeNew (world, px, py)) {
		return true;
	}
	if (aiMoveRandom (world, px, py)) {
		return true;
	}
	return false;
}

int main()
{
	uint32_t dim = 3;
	uint32_t new_win = 3;

	while (1) {
		std::cout << "Enter world size (Min = 3, Max = 30):\n";
		std::cin >> dim;
		if (std::cin.fail() || dim < 3 || dim > 30) {
			std::cerr << "Invalid world size\n" << std::endl;
			std::cin.clear ();
			std::cin.ignore ();
			continue;
		}
		break;
	}

	new_win = dim < 5 ? 3 : 5;

	sf::RenderWindow window(sf::VideoMode(512, 512), "Tic Tac Toe", sf::Style::Default);
	window.setFramerateLimit(30);
	sf::Event ev;

	World world = createWorld(dim, dim, new_win);
	initTextures("res/textures/nought.png", "res/textures/cross.png");

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
				sf::Vector2f worldIndex = sf::Vector2f(((float)mPos.x / (float)window.getSize().x) * width, 
								       ((float)mPos.y / (float)window.getSize().y) * height);
				if (!setWorldCell(&world, worldIndex.x, worldIndex.y))
					continue;
				CellType* winner = checkWin(&world, worldIndex.x, worldIndex.y);
				swapCells(&world);
				if(winner != &cellNone){
					setWinner(winner, &window);
					cleanMap(&world);
					world.lastPlaced = &cellCross;
					continue;
				}

				if(isFilled(&world))
				{
					setWinner(&cellNone, &window);
					cleanMap(&world);
				}
				uint32_t px, py;
				aiMove(&world, worldIndex.x, worldIndex.y, &px, &py);
				winner = checkWin(&world, px, py);
				swapCells(&world);
				if(winner != &cellNone){
					setWinner(winner, &window);
					cleanMap(&world);
				}

				if(isFilled(&world))
				{
					setWinner(&cellNone, &window);
					cleanMap(&world);
				}
			}
		}

		window.clear(sf::Color::White);
		drawWorld(&window, &world);
		window.display();
	}

	deleteWorld(&world);
}
