#include "Game.h"
#include "Components.h"
#include "Vec2.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Window.hpp>
#include <cmath>
#include <iostream>
#include <fstream>
#include <memory>

Game::Game(const std::string & config)
{
    init(config);
}

void Game::init(const std::string & path)
{
    readFromFile(path);
    setWindow();
    setFont();
}

void Game::run()
{
    //TODO: add pause functionality
    //some systems should work while pause some systems shouldn't
    spawnPlayer();
    while (m_running)
    {
        m_entityManager.update();
        if (!m_paused)
        {            
            sEnemySpawner();
            sMovement();
            sCollision();
            sUserInput();
            sRender();
            
        } else 
        {
            sRender();
        }
        
        m_currentFrame++;
    }
}

void Game::sUserInput()
{
    sf::Event event;
    while (m_window.pollEvent(event)) 
    {
        if (event.type == sf::Event::Closed) 
        {
            m_running = false;
            m_window.close();
        }

        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::W:                
                    m_player->cInput->up = true;
                    break;
                case sf::Keyboard::S:
                    m_player->cInput->down = true;
                    break;
                case sf::Keyboard::A:
                    m_player->cInput->left = true;
                    break;
                case sf::Keyboard::D:
                    m_player->cInput->right = true;
                    break;
                default:
                    break;
            }
        }
        if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::W:
                    m_player->cInput->up = false;
                    break;
                case sf::Keyboard::S:
                    m_player->cInput->down = false;
                    break;
                case sf::Keyboard::A:
                    m_player->cInput->left = false;
                    break;
                case sf::Keyboard::D:
                    m_player->cInput->right = false;
                    break;
                default:
                    break;
            }
        }
    }
}

void Game::sRender()
{
    m_window.clear();
    for (auto e : m_entityManager.getEntities())
    {
        e->setPosition();
        e->setRotation(e->getShapePointCount());
        m_window.draw(e->getCShape());
    }
    m_window.display();
}

void Game::setPaused(bool paused)
{
    m_paused = paused;
}

void Game::spawnPlayer()
{
    auto entity = m_entityManager.addEntity("player");
    float initialPlayerPositionX = m_window.getSize().x / 2.0f;
    float initialPlayerPositionY = m_window.getSize().y / 2.0f;
    /*TODO: need to calculate x and y components of player speed based on m_playerConfig.S
    need to add the CCollision component*/
    entity->cTransform = std::make_shared<CTransform>(
        Vec2(initialPlayerPositionX, initialPlayerPositionY), 
        calculateRandomComponentsForSpeed(m_playerConfig.S),
        m_playerConfig.V*40.0f); //defining rotating angle based on number of vertices
    entity->cShape = std::make_shared<CShape>(
        m_playerConfig.SR, 
        m_playerConfig.V, 
        sf::Color(m_playerConfig.FR,m_playerConfig.FG,m_playerConfig.FB), 
        sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), 
        m_playerConfig.OT);
    entity->cInput = std::make_shared<CInput>();
    m_player = entity;
}

void Game::spawnEnemy()
{
    auto entity = m_entityManager.addEntity("enemy");

    //TODO: needing to populate data with enemy configuration
    int amountOfVertices = rng(m_enemyConfig.VMIN, m_enemyConfig.VMAX);
    entity->cShape = std::make_shared<CShape>(
        m_enemyConfig.SR, 
        amountOfVertices, 
        sf::Color(rng(1, 255),rng(1, 255), rng(1, 255)), 
        sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), 
        m_enemyConfig.OT);


    /* added a little difference from the specification: the higher the amount of vertices, 
    higher the speed of the enemy */
    entity->cTransform = std::make_shared<CTransform>( 
        generateValidStartingPosition(m_enemyConfig.CR), 
        calculateRandomComponentsForSpeed(amountOfVertices),
        amountOfVertices*40.0f); //defining rotating angle based on number of vertices

    //TODO: need to create the CScore and CLifespan components

    entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

    entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, 
    amountOfVertices, 
    sf::Color(rng(0, 255), rng(0, 255), rng(0, 255)),
    sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),
    m_enemyConfig.OT);

    m_lastEnemySpawnTime = m_currentFrame;
}

Vec2 Game::generateValidStartingPosition(int collisionRadius)
{
    int possiblePosition_x = rng(collisionRadius, m_window.getSize().x - collisionRadius);
    int possiblePosition_y = rng(collisionRadius, m_window.getSize().y - collisionRadius);

    return Vec2(possiblePosition_x, possiblePosition_y);
}

Vec2 Game::calculateRandomComponentsForSpeed(int speed)
{
    int randomness_x = rng(-1, 1);
    int randomness_y = rng(-1, 1);
    if (randomness_x == randomness_y == 0) {
        return Vec2(std::sqrt(speed*speed/2), std::sqrt(speed*speed/2));
    }
    return Vec2(randomness_x*std::sqrt(speed*speed/2), randomness_y*std::sqrt(speed*speed/2));
}

int Game::rng(int min, int max)
{
    return rand()%(max-min + 1) + min;
}

void Game::readFromFile(const std::string & path)
{
    std::ifstream fin(path);
    std::string line, identifier;
     if (fin.is_open()) {
          while (fin.good()) {
               fin >> line;
               std::cout << line;               
     
                  if (line == "Window") {
                      fin >> m_w >> m_h >> m_fl >> m_fs;                              
                  }
                  else if (line == "Font") {
                      fin >> m_fontFile >> m_fontSize >> m_fontR >> m_fontG >> m_fontB;          
                  }
                  else if (line == "Player") {
                      fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR 
                          >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG 
                          >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
                  }
                  else if (line == "Enemy") {
                      fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX 
                          >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT 
                          >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
                  }
                  else if (line == "Bullet") {
                      fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR 
                          >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG 
                          >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
                  }
          }
     }
    fin.close();
}

void Game::setWindow()
{
    if (m_fs == 1)
    {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();    
        m_window.create(desktop, "Joguin", sf::Style::Fullscreen);
    } else 
    {
        m_window.create(sf::VideoMode(m_w, m_h), "Joguin");
    }
    m_window.setFramerateLimit(m_fl);    
}

void Game::setFont()
{
    m_font.loadFromFile(m_fontFile);    
    if (!m_font.loadFromFile(m_fontFile))
    {
        return;
    }
}

void Game::sMovement()
{
    movePlayer();
    moveEnemy();
}

void Game::sEnemySpawner() 
{
    if ( m_currentFrame - m_lastEnemySpawnTime >= INTERVAL_OF_FRAMES_TO_SPAWN_ENEMY) {
        spawnEnemy();
    }
}

void Game::movePlayer() {
    m_player->cTransform->velocity = {0,0};
    if ( m_player->cInput->up)
    {
         m_player->cTransform->velocity.y = -7;
    }
    if ( m_player->cInput->down)
    {
         m_player->cTransform->velocity.y = 7;
    }
    if ( m_player->cInput->right)
    {
         m_player->cTransform->velocity.x = 7;
    }
    if ( m_player->cInput->left)
    {
         m_player->cTransform->velocity.x = -7;
    }
     m_player->cTransform->pos.y +=  m_player->cTransform->velocity.y;
     m_player->cTransform->pos.x +=  m_player->cTransform->velocity.x;
}

void Game::moveEnemy() {    
    for (auto e : m_entityManager.getEntities())
    {
        if (e->tag() != "player") {
            e->cTransform->pos.y +=  e->cTransform->velocity.y;
            e->cTransform->pos.x +=  e->cTransform->velocity.x;
        }
    }
}

void Game::sCollision() {
    windowCollision();
}

void Game::windowCollision() {
    std::cout << "oe";
    for (auto e : m_entityManager.getEntities())
    {
        if (e->tag() != "player") {
            //TODO: think about the collision code, couldnt understand what was done here before         
        }
    }
}