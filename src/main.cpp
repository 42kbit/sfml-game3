#include "SFML/Graphics.hpp"

int main()
{
    sf::RenderWindow window(sf::VideoMode(500, 500), "Tic Tac Toe", sf::Style::Default);
    sf::Event ev;

    while(window.isOpen())
    {
        while(window.pollEvent(ev))
        {
            if(ev.type == sf::Event::Closed) 
                window.close();
        }

        window.clear();

        window.display();
    }
}