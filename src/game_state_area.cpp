#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "game_state_area.hpp"
#include "game_state_menu_status.hpp"
#include "game_state_menu_items.hpp"

#include "treasure_chest.hpp"

void GameStateArea::handleEvent(sf::Event& event)
{
	if(subState == SubState::GAME)
	{
		if(event.type == sf::Event::KeyPressed)
		{
			// Open the start menu
			if(event.key.code == sf::Keyboard::Return)
			{
				subState = SubState::START;
				startMenu.select(0, '*');
			}
			// Activate adjacent objects
			if(event.key.code == sf::Keyboard::Space)
			{
				// Search for nearby chests
				for(auto& chest : area->chests)
				{
					sf::Vector2f p1 = chest.getPosition();
					sf::Vector2f p2 = player->mover->getPosition();
					sf::Vector2i p1i(static_cast<int>(p1.x), static_cast<int>(p1.y));
					sf::Vector2i p2i(static_cast<int>(p2.x), static_cast<int>(p2.y));

					switch(player->mover->getFacing())
					{
						case Direction::NORTH:
						if(p1i.x == p2i.x && p1i.y == p2i.y-1)
							openChest(chest);
						break;

						case Direction::EAST:
						if(p1i.y == p2i.y && p1i.x == p2i.x+1)
							openChest(chest);
						break;

						case Direction::SOUTH:
						if(p1i.x == p2i.x && p1i.y == p2i.y+1)
							openChest(chest);
						break;

						case Direction::WEST:
						if(p1i.y == p2i.y && p1i.x == p2i.x-1)
							openChest(chest);
						break;

						default:
						break;
					}
				}
			}
		}
	}
	else if(subState == SubState::START)
	{
		if(event.type == sf::Event::KeyPressed)
		{
			// Handle menu navigation
			if(event.key.code == sf::Keyboard::Up)
				startMenu.navigate(gui::Direction::UP, gui::NavigationMode::STOP, gui::NavigationMode::LOOP);
			else if(event.key.code == sf::Keyboard::Right)
				startMenu.navigate(gui::Direction::RIGHT, gui::NavigationMode::STOP, gui::NavigationMode::LOOP);
			else if(event.key.code == sf::Keyboard::Down)
				startMenu.navigate(gui::Direction::DOWN, gui::NavigationMode::STOP, gui::NavigationMode::LOOP);
			else if(event.key.code == sf::Keyboard::Left)
				startMenu.navigate(gui::Direction::LEFT, gui::NavigationMode::STOP, gui::NavigationMode::LOOP);
			// Open a menu option
			else if(event.key.code == sf::Keyboard::Return ||
				event.key.code == sf::Keyboard::Space)
				startMenu.activate(this);
			// Close the menu
			else if(event.key.code == sf::Keyboard::Escape)
			{
				subState = SubState::GAME;
			}
		}
	}
	else if(subState == SubState::INFO)
	{
		if(event.type == sf::Event::KeyPressed)
		{
			if(event.key.code == sf::Keyboard::Return ||
				event.key.code == sf::Keyboard::Space)
			{
				if(infoMsgBox.getPage() == infoMsgBox.numPages()-1)
					subState = SubState::GAME;
				else
					infoMsgBox.setPage(infoMsgBox.getPage()+1);
			}
		}
	}
}

void GameStateArea::handleInput(float dt)
{
	if(subState == SubState::GAME)
	{
		// Handle player movement	
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			this->player->step(dt, Direction::NORTH, this->area->tilemap);
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			this->player->step(dt, Direction::EAST, this->area->tilemap);
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			this->player->step(dt, Direction::SOUTH, this->area->tilemap);
		else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			this->player->step(dt, Direction::WEST, this->area->tilemap);
		else
			this->player->step(dt, Direction::NONE, this->area->tilemap);
	}
}

void GameStateArea::update(float dt)
{
	if(subState == SubState::GAME)
	{
		this->player->update(dt);
		for(auto& chest : this->area->chests)
		{
			chest.update(dt);
		}
	}
}

void GameStateArea::draw(sf::RenderWindow& window, float dt) const
{
	window.setView(this->view);
	window.draw(this->area->tilemap);
	for(auto& chest : this->area->chests)
	{
		window.draw(*chest.renderer);
	}
	window.draw(*this->player->renderer);
	if(subState == SubState::START)
	{
		window.draw(startMenu);
	}
	else if(subState == SubState::INFO)
	{
		window.draw(infoMsgBox);
	}
}

void GameStateArea::callbackItems(size_t index)
{
	std::shared_ptr<GameState> prevState = state;
	state.reset(new GameStateMenuItems(state, prevState, player));
}
void GameStateArea::callbackStatus(size_t index)
{
	std::shared_ptr<GameState> prevState = state;
	state.reset(new GameStateMenuStatus(state, prevState, player));
}
void GameStateArea::callbackSave(size_t index)
{
	player->save(mgr);
	infoMsgBox.setText("Saved game.");
	subState = SubState::INFO;
}
void GameStateArea::callbackExit(size_t index)
{
	subState = SubState::GAME;
}

void GameStateArea::openChest(TreasureChest& chest)
{
	if(chest.getClosed())
	{
		std::string str = "Found\n";
		for(size_t i = 0; i < chest.inventory.size(); ++i)
		{
			Item* item = chest.inventory.get(i);
			str += item->name + " x " + std::to_string(chest.inventory.count(item)) + "\n";
		}
		infoMsgBox.setText(str);
		subState = SubState::INFO;
		chest.toggle(*player);
	}
}
