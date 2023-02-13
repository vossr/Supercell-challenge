#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <filesystem>
#include <iostream>

#include "resources/Resources.h"
#include "ControllerInput.h"
#include "ControllerAI.h"
#include "Paddle.h"
#include "Pitch.h"
#include "Game.h"
#include "Ball.h"

Game::Game()
	: m_pPitch(std::make_unique<Pitch>(this)), m_pBall(std::make_unique<Ball>(this)), m_state(State::WAITING), m_pClock(std::make_unique<sf::Clock>()), music(std::make_unique<sf::Music>()), wobble(std::make_unique<sf::Shader>())
{
	m_pPaddles[Side::LEFT] = std::make_unique<Paddle>(this);
	m_pPaddles[Side::RIGHT] = std::make_unique<Paddle>(this);

	m_controllers[Side::LEFT] = std::make_unique<ControllerInput>(this, m_pPaddles[0].get());
	m_controllers[Side::RIGHT] = std::make_unique<ControllerAI>(this, m_pPaddles[1].get());

	m_score[Side::LEFT] = 0;
	m_score[Side::RIGHT] = 0;
}

Game::~Game()
{
}

bool Game::initialise(sf::Vector2f pitchSize)
{
	if (!m_pPitch->initialise(pitchSize))
		return false;

	if (!m_pPaddles[Side::LEFT]->initialise(Side::LEFT))
		return false;
	if (!m_pPaddles[Side::RIGHT]->initialise(Side::RIGHT))
		return false;
	if (!m_controllers[Side::LEFT]->initialise())
		return false;
	if (!m_controllers[Side::RIGHT]->initialise())
		return false;
	if (!m_pBall->initialise())
		return false;

	m_pClock->restart();

	std::string assetPath = Resources::getAssetPath();
	if (!m_font.loadFromFile(assetPath + "Lavigne.ttf"))
	{
		std::cerr << "Unable to load font" << std::endl;
		return false;
	}

	if (!music->openFromFile("assets/synthwave80s.ogg"))
	{
		std::cout << "music failed to load\n";
	}
	music->setVolume(2); // reduce the volume
	music->setLoop(true);
	music->play();

	if (!wobble->loadFromFile("assets/wobble.frag", sf::Shader::Fragment))
		;

	return true;
}

void Game::update(float deltaTime)
{
	switch (m_state)
	{
	case State::WAITING:
	{
		if (m_pClock->getElapsedTime().asSeconds() >= 3.f)
		{
			m_pBall->fireFromCenter();
			m_state = State::ACTIVE;
		}
		break;
	}
	case State::ACTIVE:
		break;
	}
	m_pPitch->update(deltaTime);
	m_pPaddles[Side::LEFT]->update(deltaTime);
	m_pPaddles[Side::RIGHT]->update(deltaTime);
	m_controllers[Side::LEFT]->update(deltaTime);
	m_controllers[Side::RIGHT]->update(deltaTime);
	m_pBall->update(deltaTime);
}

void Game::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	target.draw(*m_pPitch.get());
	target.draw(*m_pPaddles[Side::LEFT].get());
	target.draw(*m_pPaddles[Side::RIGHT].get());
	target.draw(*m_pBall.get());

	// wobble.setUniform("iTime", clock.getElapsedTime().asSeconds());

	// draw the score
	int center = target.getSize().x / 2.0;

	sf::Text Score;
	Score.setFont(m_font);
	Score.setStyle(sf::Text::Bold);
	Score.setCharacterSize(100);

	Score.setPosition(center - 150, 10.f);
	Score.setFillColor(sf::Color(0xffffff50));
	Score.setString(std::to_string(m_score[Side::LEFT]));
	target.draw(Score);

	Score.setPosition(center + 90, 10.f);
	Score.setFillColor(sf::Color(0xffffff50));
	Score.setString(std::to_string(m_score[Side::RIGHT]));
	target.draw(Score);
}

void Game::scoreGoal(Side side)
{
	m_state = State::WAITING;
	m_pClock->restart();
	m_score[side]++;
	m_controllers[Side::RIGHT]->initialise();
}

void Game::onKeyPressed(sf::Keyboard::Key key)
{
	m_controllers[Side::LEFT]->onKeyPressed(key);
	m_controllers[Side::RIGHT]->onKeyPressed(key);
}

void Game::onKeyReleased(sf::Keyboard::Key key)
{
	m_controllers[Side::LEFT]->onKeyReleased(key);
	m_controllers[Side::RIGHT]->onKeyReleased(key);
}
