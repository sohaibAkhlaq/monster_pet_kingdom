#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
// ==================== BUTTON CLASS ==================== //

// This class makes a clickable button with text, which changes color when hovered or clicked.
// It uses encapsulation by keeping button state and appearance details hidden inside the class.
class Button
{
private:
    sf::RectangleShape shape;
    sf::Color normalColor;
    sf::Color hoverColor;
    sf::Color activeColor;
    bool isHovered;
    bool isActive;
    sf::Text text;

public:
    Button() : shape(), text() {}                                             // default constructor
    Button(const sf::Font &font, const std::string &label, unsigned int size, // parametrized constructor
           sf::Color normal, sf::Color hover, sf::Color active) : normalColor(normal), hoverColor(hover), activeColor(active),
                                                                  isHovered(false), isActive(false)
    {
        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(size);
        text.setFillColor(sf::Color::Black);

        float padding = 20.0f;
        shape.setSize(sf::Vector2f(text.getLocalBounds().width + padding * 2,
                                   text.getLocalBounds().height + padding));
        shape.setFillColor(normalColor);
        shape.setOutlineThickness(2.0f);
        shape.setOutlineColor(sf::Color::Black);
    }

    void setPosition(float x, float y)
    {
        shape.setPosition(x, y);
        text.setPosition(x + (shape.getSize().x - text.getLocalBounds().width) / 2,
                         y + (shape.getSize().y - text.getLocalBounds().height) / 2 - 5);
    }

    void update(const sf::Vector2f &mousePos)
    {
        isHovered = shape.getGlobalBounds().contains(mousePos);

        if (isActive)
        {
            shape.setFillColor(activeColor);
            text.setFillColor(sf::Color::White);
        }
        else if (isHovered)
        {
            shape.setFillColor(hoverColor);
            text.setFillColor(sf::Color::Black);
        }
        else
        {
            shape.setFillColor(normalColor);
            text.setFillColor(sf::Color::Black);
        }
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(shape);
        window.draw(text);
    }

    void setActive(bool active)
    {
        isActive = active;
    }
                                                // getter
    bool contains(const sf::Vector2f &point) const
    {
        return shape.getGlobalBounds().contains(point);
    }

    sf::FloatRect getBounds() const
    {
        return shape.getGlobalBounds();
    }

    sf::Vector2f getPosition() const { return shape.getPosition(); }

    sf::Text &getText() { return text; }
};

// ==================== PET CLASS ==================== //

// This class defines a game pet that has stats like HP, speed, and level, and can level up with training points.
// It uses encapsulation to manage pet data and behavior, and polymorphism so different pets can customize setup and texture.
class Pet
{
protected:
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Text nameText;
    sf::Text infoText;
    sf::RectangleShape background;
    sf::RectangleShape highlight;
    std::string name;
    std::string type;
    std::string ability;
    int level;
    int hp;
    int attack;
    int speed;
    int trainingPoints;
    sf::Vector2f position;
    bool isSelected;
    static constexpr float IMAGE_SIZE = 180.f;

    const int MAX_LEVEL = 3;

    int baseHP;
    int baseAttack;
    int baseSpeed;
    float hpGrowth;
    float attackGrowth;
    float speedGrowth;

    void removeWhiteBackground(sf::Image &image)
    {
        bool hasTransparency = false;
        for (unsigned int y = 0; y < image.getSize().y; ++y)
        {
            for (unsigned int x = 0; x < image.getSize().x; ++x)
            {
                sf::Color pixel = image.getPixel(x, y);
                if (pixel.r > 200 && pixel.g > 200 && pixel.b > 200)
                {
                    pixel.a = 0;
                    image.setPixel(x, y, pixel);
                    hasTransparency = true;
                }
            }
        }
        if (!hasTransparency)
        {
            image.createMaskFromColor(sf::Color::White);
        }
    }

    void updateStatsText()
    {
        std::string stats =
            "Type:      " + type + "\n" +
            "Ability:   " + ability + "\n" +
            "Level:     " + std::to_string(level) +
            " (TP: " + std::to_string(trainingPoints) +
            "/" + std::to_string(calculateRequiredTP()) + ")\n" +
            "HP:        " + std::to_string(hp) + " (+" +
            std::to_string(static_cast<int>(baseHP * pow(hpGrowth, level - 1) - baseHP)) + ")\n" +
            "Attack:    " + std::to_string(attack) + " (+" +
            std::to_string(static_cast<int>(baseAttack * pow(attackGrowth, level - 1) - baseAttack)) + ")\n" +
            "Speed:     " + std::to_string(speed) + " (+" +
            std::to_string(static_cast<int>(baseSpeed * pow(speedGrowth, level - 1) - baseSpeed)) + ")";

        infoText.setString(stats);
    }

public:
    Pet() : level(1), trainingPoints(0), // constructor
            baseHP(100), baseAttack(10), baseSpeed(5),
            hpGrowth(1.2f), attackGrowth(1.1f), speedGrowth(1.05f)
    {
        updateStats();
    }

    virtual ~Pet() {} // polymorphism used

    virtual void setup(const sf::Font &font) = 0;

    virtual std::string getTexturePath() const = 0;

    void updateStats()
    {
        hp = static_cast<int>(baseHP * pow(hpGrowth, level - 1));
        attack = static_cast<int>(baseAttack * pow(attackGrowth, level - 1));
        speed = static_cast<int>(baseSpeed * pow(speedGrowth, level - 1));
    }

    void setPosition(float posX, float posY)
    {
        position.x = posX;
        position.y = posY;
        background.setPosition(position);
        highlight.setPosition(position.x - 5, position.y - 5);

        float imageX = position.x + 20.f;
        float imageY = position.y + (background.getSize().y - sprite.getGlobalBounds().height) / 2;
        sprite.setPosition(imageX, imageY);

        float textX = position.x + 180.f + 30.f;
        nameText.setPosition(textX, position.y + 20);
        infoText.setPosition(textX, position.y + 60);
    }

    void gainExperience(int amount)
    {
        int requiredTP = calculateRequiredTP();
        trainingPoints += amount;

        while (trainingPoints >= requiredTP && level < MAX_LEVEL)
        {
            levelUp();
            trainingPoints -= requiredTP;
            requiredTP = calculateRequiredTP();
        }
    }

    void levelUp()
    {
        level++;
        updateStats();
    }

    int calculateRequiredTP() const
    {
        return static_cast<int>(100 * pow(1.2, level - 1));
    }
                                                                    // getter
    sf::Vector2f getPosition() const { return position; }
    void setSelected(bool selected) { isSelected = selected; }
    sf::FloatRect getBounds() const { return background.getGlobalBounds(); }
    std::string getName() const { return name; }
    std::string getType() const { return type; }
    std::string getAbility() const { return ability; }
    int getLevel() const { return level; }
    int getHP() const { return hp; }
    int getAttack() const { return attack; }
    int getSpeed() const { return speed; }
    int getTrainingPoints() const { return trainingPoints; }
    sf::Sprite &getSprite() { return sprite; }

    virtual void draw(sf::RenderWindow &window)
    {
        window.draw(background);
        window.draw(sprite);
        window.draw(nameText);
        window.draw(infoText);

        if (isSelected)
        {
            window.draw(highlight);
        }
    }

protected:
    void setupCommon(const sf::Font &font, const std::string &textureFile)
    {
        if (!texture.loadFromFile(textureFile))
        {
            std::cerr << "Error loading pet texture: " << textureFile << std::endl;
            sf::Image placeholder;
            placeholder.create(64, 64, sf::Color::Magenta);
            texture.loadFromImage(placeholder);
        }
        else
        {
            sf::Image img = texture.copyToImage();
            removeWhiteBackground(img);
            texture.loadFromImage(img);
        }

        texture.setSmooth(true);
        sprite.setTexture(texture, true);

        const float BOX_WIDTH = 450.f;
        const float BOX_HEIGHT = 250.f;
        const float IMAGE_AREA_WIDTH = 180.f;
        const float PADDING = 20.f;

        sf::FloatRect spriteBounds = sprite.getLocalBounds();
        float scale = std::min(
            (IMAGE_AREA_WIDTH - 2 * PADDING) / spriteBounds.width,
            (BOX_HEIGHT - 2 * PADDING) / spriteBounds.height);
        sprite.setScale(scale, scale);

        background.setSize(sf::Vector2f(BOX_WIDTH, BOX_HEIGHT));
        background.setFillColor(sf::Color(30, 30, 50, 240));
        background.setOutlineThickness(3.f);
        background.setOutlineColor(sf::Color(255, 200, 100));

        highlight.setSize(sf::Vector2f(BOX_WIDTH + 10, BOX_HEIGHT + 10));
        highlight.setFillColor(sf::Color::Transparent);
        highlight.setOutlineThickness(3.f);
        highlight.setOutlineColor(sf::Color(255, 100, 100, 200));

        nameText.setFont(font);
        nameText.setString(name);
        nameText.setStyle(sf::Text::Bold | sf::Text::Underlined);
        nameText.setCharacterSize(34);
        nameText.setFillColor(sf::Color(255, 225, 150));
        nameText.setOutlineThickness(2.f);
        nameText.setOutlineColor(sf::Color::Black);

        infoText.setFont(font);
        infoText.setCharacterSize(20);
        infoText.setFillColor(sf::Color(220, 240, 255));
        infoText.setOutlineThickness(1.f);
        infoText.setOutlineColor(sf::Color::Black);
        infoText.setLineSpacing(1.2f);
        updateStatsText();
    }
};

// --------------- Dragon Class ----------------//

// This class creates a specific pet called Dragon with fire type and special fireball ability.
// It uses inheritance to extend the Pet class and override setup details like texture and stats.
class Dragon : public Pet
{
public:
    Dragon()                // constructor
    {
        name = "Dragon";
        type = "Fire";
        ability = "Fireball";
        baseHP = 120;
        baseAttack = 15;
        baseSpeed = 4;
        hpGrowth = 1.25f;
        attackGrowth = 1.15f;
        speedGrowth = 1.03f;
        updateStats();
    }

    void setup(const sf::Font &font) override
    {
        setupCommon(font, "dragon1.png");
    }

    std::string getTexturePath() const override { return "dragon1.png"; }               // getter
};

//-------- Phoneix Class --------------------//

// This class creates a specific pet Phoneix with ice type and special freeze ability.
// It uses inheritance to extend the Pet class and override setup details like texture and stats.
class Phoenix : public Pet
{
public:
    Phoenix()                                           // constructor
    {
        name = "Phoenix";
        type = "Ice";
        ability = "Freeze";
        baseHP = 90;
        baseAttack = 12;
        baseSpeed = 7;
        hpGrowth = 1.15f;
        attackGrowth = 1.1f;
        speedGrowth = 1.08f;
        updateStats();
    }
    void setup(const sf::Font &font) override
    {
        setupCommon(font, "phoneix1.png");
    }
    // getter
    std::string getTexturePath() const override { return "phoneix1.png"; }
};

//---------- Griffin Class --------------//

// This class creates a specific pet cGriffin with electric type and special flightning ability.
// It uses inheritance to extend the Pet class and override setup details like texture and stats.
class Griffin : public Pet
{
public:
    Griffin() // constructor
    {
        name = "Griffin";
        type = "Electric";
        ability = "Lightning";
        baseHP = 100;
        baseAttack = 10;
        baseSpeed = 6;
        hpGrowth = 1.15f;
        attackGrowth = 1.1f;
        speedGrowth = 1.00f;
        updateStats();
    }
    void setup(const sf::Font &font) override
    {
        setupCommon(font, "griffin1.png");
    }

    std::string getTexturePath() const override { return "griffin1.png"; } // getter
};

//------------ Unicorn Class ---------------//

// This class creates a specific pet Unicorn with magic type and special healing ability.
// It uses inheritance to extend the Pet class and override setup details like texture and stats.
class Unicorn : public Pet
{
public:
    Unicorn()                               // constructor
    {
        name = "Unicorn";
        type = "Magic";
        ability = "Healing";
        baseHP = 110;
        baseAttack = 8;
        baseSpeed = 5;
        hpGrowth = 1.15f;
        attackGrowth = 1.1f;
        speedGrowth = 1.1f;
        updateStats();
    }
    void setup(const sf::Font &font) override
    {
        setupCommon(font, "unicorn1.png");
    }

    std::string getTexturePath() const override { return "unicorn1.png"; } // getter
};

// ==================== PET SELECTION WINDOW CLASS==================== //

// This class shows a window where players can pick one pet from a list of options.
// It uses composition and dynamic memory, managing buttons, pets, and user interaction.
class PetSelectionWindow
{
private:
    static const int MAX_PETS = 4;
    sf::RectangleShape window;
    sf::RectangleShape backgroundDim;
    sf::Text title;
    Button closeButton;
    Button selectButton;
    Pet *pets[MAX_PETS];
    bool isActive;
    sf::Font font;
    int selectedPet;
    int petCount;

    void initializePets()
    {
        pets[0] = new Dragon();
        pets[1] = new Phoenix();
        pets[2] = new Griffin();
        pets[3] = new Unicorn();
        petCount = MAX_PETS;
    }

    void cleanupPets()
    {
        for (int i = 0; i < petCount; i++)
        {
            delete pets[i];
            pets[i] = nullptr;
        }
        petCount = 0;
    }

public:
    PetSelectionWindow() : isActive(false), selectedPet(-1), petCount(0) // constructor
    {
        initializePets();
    }

    ~PetSelectionWindow()                           // destructor
    {
        cleanupPets();
    }

    void setup(const sf::Font &gameFont)
    {
        font = gameFont;

        backgroundDim.setFillColor(sf::Color(0, 0, 0, 180));

        const float BOX_WIDTH = 500.f;
        const float BOX_HEIGHT = 300.f;
        const float HORIZONTAL_SPACING = 50.f;
        const float VERTICAL_SPACING = 50.f;
        const float BOTTOM_PADDING = 50.f;

        window.setSize(sf::Vector2f(
            2 * BOX_WIDTH + 3 * HORIZONTAL_SPACING,
            2 * BOX_HEIGHT + 3 * VERTICAL_SPACING + BOTTOM_PADDING));
        window.setFillColor(sf::Color(40, 40, 50, 240));
        window.setOutlineThickness(4.f);
        window.setOutlineColor(sf::Color(255, 215, 0));

        title.setFont(font);
        title.setString("SELECT A PET FOR TRAINING");
        title.setCharacterSize(48);
        title.setFillColor(sf::Color(255, 215, 0));
        title.setStyle(sf::Text::Bold);
        title.setOutlineThickness(2.f);
        title.setOutlineColor(sf::Color::Black);

        closeButton = Button(font, "CANCEL", 32,
                             sf::Color(255, 100, 100),
                             sf::Color(255, 150, 150),
                             sf::Color(200, 50, 50));

        selectButton = Button(font, "SELECT PET", 32,
                              sf::Color(100, 200, 100),
                              sf::Color(150, 250, 150),
                              sf::Color(50, 150, 50));

        for (int i = 0; i < petCount; i++)
        {
            pets[i]->setup(font);
        }

        float startX = HORIZONTAL_SPACING;
        float startY = VERTICAL_SPACING + 50;

        pets[0]->setPosition(startX, startY);
        pets[1]->setPosition(startX + BOX_WIDTH + HORIZONTAL_SPACING, startY);
        pets[2]->setPosition(startX, startY + BOX_HEIGHT + VERTICAL_SPACING);
        pets[3]->setPosition(startX + BOX_WIDTH + HORIZONTAL_SPACING,
                             startY + BOX_HEIGHT + VERTICAL_SPACING);

        float closeButtonX = (window.getSize().x - closeButton.getBounds().width) / 2 - 120;
        float selectButtonX = (window.getSize().x - selectButton.getBounds().width) / 2 + 120;
        float buttonsY = window.getSize().y - BOTTOM_PADDING + 30;

        closeButton.setPosition(closeButtonX, buttonsY);
        selectButton.setPosition(selectButtonX, buttonsY);
    }

    void open()
    {
        isActive = true;
        selectedPet = -1;
        for (int i = 0; i < petCount; i++)
        {
            pets[i]->setSelected(false);
        }
    }

    void close() { isActive = false; }

    bool isOpen() const { return isActive; }

    void update(const sf::Vector2f &mousePos)
    {
        closeButton.update(mousePos);
        selectButton.update(mousePos);

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            bool clickedOnPet = false;
            for (int i = 0; i < petCount; i++)
            {
                if (pets[i]->getBounds().contains(mousePos))
                {
                    clickedOnPet = true;
                    break;
                }
            }
            if (!clickedOnPet)
            {
                selectedPet = -1;
            }
        }

        for (int i = 0; i < petCount; i++)
        {
            pets[i]->setSelected(false);
            if (pets[i]->getBounds().contains(mousePos))
            {
                selectedPet = i;
                pets[i]->setSelected(true);
            }
        }

        selectButton.setActive(selectedPet != -1);
    }

    void handleInput(const sf::Event &event, const sf::Vector2f &mousePos)
    {
        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left)
        {

            if (closeButton.contains(mousePos))
            {
                close();
            }
            else
            {

                for (int i = 0; i < petCount; i++)
                {
                    if (pets[i]->getBounds().contains(mousePos))
                    {
                        selectedPet = i;
                        close();
                        return;
                    }
                }
            }
        }
    }

    void draw(sf::RenderWindow &targetWindow)
    {
        if (!isActive)
            return;

        sf::Vector2u winSize = targetWindow.getSize();
        backgroundDim.setSize(sf::Vector2f(winSize.x, winSize.y));
        targetWindow.draw(backgroundDim);

        sf::FloatRect windowBounds = window.getLocalBounds();
        window.setPosition((winSize.x - windowBounds.width) / 2,
                           (winSize.y - windowBounds.height) / 2);

        title.setPosition(window.getPosition().x + (windowBounds.width - title.getLocalBounds().width) / 2,
                          window.getPosition().y + 30);
        targetWindow.draw(window);
        targetWindow.draw(title);

        for (int i = 0; i < petCount; i++)
        {
            sf::Vector2f originalPos = pets[i]->getPosition();
            pets[i]->setPosition(window.getPosition().x + originalPos.x,
                                 window.getPosition().y + originalPos.y);
            pets[i]->draw(targetWindow);
            pets[i]->setPosition(originalPos.x, originalPos.y);
        }

        closeButton.draw(targetWindow);
        selectButton.draw(targetWindow);
    }

    Pet *getSelectedPet()
    {
        if (selectedPet >= 0 && selectedPet < petCount)
        {
            return pets[selectedPet];
        }
        return nullptr;
    }
};

// ==================== PET DISPLAY CLASS ==================== //

// This class manages a display for showing a collection of pets, allowing you to view details or close the window.
// It uses polymorphism to handle different pet types (like Dragon, Phoenix, etc.) that inherit from a common Pet base class.
class PetDisplay
{
private:
    static const int MAX_PETS = 4;

    sf::RectangleShape window;
    sf::RectangleShape backgroundDim;
    sf::Text title;
    Button closeButton;
    Button viewButton;
    Pet *pets[MAX_PETS];
    bool isActive;
    sf::Font font;
    int selectedPet;
    int petCount;

    void initializePets()
    {
        pets[0] = new Dragon();
        pets[1] = new Phoenix();
        pets[2] = new Griffin();
        pets[3] = new Unicorn();
        petCount = MAX_PETS;
    }

    void cleanupPets()
    {
        for (int i = 0; i < petCount; i++)
        {
            delete pets[i];
            pets[i] = nullptr;
        }
        petCount = 0;
    }

public:
    PetDisplay() : isActive(false), selectedPet(-1), petCount(0) // constructor
    {
        initializePets();
    }

    ~PetDisplay() // destructor
    {
        cleanupPets();
    }

    void setup(const sf::Font &gameFont)
    {
        font = gameFont;

        backgroundDim.setFillColor(sf::Color(0, 0, 0, 180));

        const float BOX_WIDTH = 470.f;
        const float BOX_HEIGHT = 250.f;
        const float HORIZONTAL_SPACING = 50.f;
        const float VERTICAL_SPACING = 50.f;
        const float BOTTOM_PADDING = 50.f;

        window.setSize(sf::Vector2f(
            2 * BOX_WIDTH + 3 * HORIZONTAL_SPACING,
            2 * BOX_HEIGHT + 3 * VERTICAL_SPACING + BOTTOM_PADDING));
        window.setFillColor(sf::Color(40, 40, 50, 240));
        window.setOutlineThickness(4.f);
        window.setOutlineColor(sf::Color(255, 215, 0));

        title.setFont(font);
        title.setString("YOUR PET COLLECTION");
        title.setCharacterSize(48);
        title.setFillColor(sf::Color(255, 215, 0));
        title.setStyle(sf::Text::Bold);
        title.setOutlineThickness(2.f);
        title.setOutlineColor(sf::Color::Black);

        closeButton = Button(font, "CLOSE", 32,
                             sf::Color(255, 100, 100),
                             sf::Color(255, 150, 150),
                             sf::Color(200, 50, 50));

        for (int i = 0; i < petCount; i++)
        {
            pets[i]->setup(font);
        }

        float startX = HORIZONTAL_SPACING;
        float startY = VERTICAL_SPACING + 50;

        pets[0]->setPosition(startX, startY);
        pets[1]->setPosition(startX + BOX_WIDTH + HORIZONTAL_SPACING, startY);
        pets[2]->setPosition(startX, startY + BOX_HEIGHT + VERTICAL_SPACING);
        pets[3]->setPosition(startX + BOX_WIDTH + HORIZONTAL_SPACING,
                             startY + BOX_HEIGHT + VERTICAL_SPACING);

        float closeButtonX = (window.getSize().x - closeButton.getBounds().width) / 2 - 120;
        float viewButtonX = (window.getSize().x - viewButton.getBounds().width) / 2 + 120;
        float buttonsY = window.getSize().y - BOTTOM_PADDING + 30;

        closeButton.setPosition(closeButtonX, buttonsY);
        viewButton.setPosition(viewButtonX, buttonsY);
    }

    void refresh(const sf::Font &font)
    {
        setup(font);
    }

    void open()
    {
        isActive = true;
        selectedPet = -1;
        for (int i = 0; i < petCount; i++)
        {
            pets[i]->setSelected(false);
        }
    }

    void close() { isActive = false; }

    bool isOpen() const { return isActive; }

    void update(const sf::Vector2f &mousePos)
    {
        closeButton.update(mousePos);

        for (int i = 0; i < petCount; i++)
        {
            pets[i]->setSelected(pets[i]->getBounds().contains(mousePos));
        }

        for (int i = 0; i < petCount; i++)
        {
            pets[i]->setSelected(false);
            if (pets[i]->getBounds().contains(mousePos))
            {
                selectedPet = i;
                pets[i]->setSelected(true);
            }
        }

        viewButton.setActive(selectedPet != -1);
    }

    void handleInput(const sf::Event &event, const sf::Vector2f &mousePos)
    {
        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left)
        {

            if (closeButton.contains(mousePos))
            {
                close();
            }
            else if (viewButton.contains(mousePos) && selectedPet != -1)
            {
                std::cout << "Viewing: " << pets[selectedPet]->getName() << std::endl;
            }
        }
    }

    void draw(sf::RenderWindow &targetWindow)
    {
        if (!isActive)
            return;

        sf::Vector2u winSize = targetWindow.getSize();
        backgroundDim.setSize(sf::Vector2f(winSize.x, winSize.y));
        targetWindow.draw(backgroundDim);

        sf::FloatRect windowBounds = window.getLocalBounds();
        window.setPosition((winSize.x - windowBounds.width) / 2,
                           (winSize.y - windowBounds.height) / 2);

        title.setPosition(window.getPosition().x + (windowBounds.width - title.getLocalBounds().width) / 2,
                          window.getPosition().y + 30);
        targetWindow.draw(window);
        targetWindow.draw(title);

        for (int i = 0; i < petCount; i++)
        {
            sf::Vector2f originalPos = pets[i]->getPosition();
            pets[i]->setPosition(window.getPosition().x + originalPos.x,
                                 window.getPosition().y + originalPos.y);
            pets[i]->draw(targetWindow);
            pets[i]->setPosition(originalPos.x, originalPos.y);
        }

        closeButton.draw(targetWindow);
        viewButton.draw(targetWindow);
    }

    Pet *getSelectedPet()
    {
        if (selectedPet >= 0 && selectedPet < petCount)
        {
            return pets[selectedPet];
        }
        return nullptr;
    }
};

// ==================== USER DATA CLASS ==================== //

// This class handles user data like username, diamonds, pets, and items, loading and saving it to a file.
// It uses concepts like encapsulation (keeping data private) and file I/O to save and load user information.
class UserData
{
private:
    std::string filename;
    std::string username;
    int diamonds;
    std::string pets[4];
    int itemQuantities[5];
    std::string itemNames[5] = {
        "Healing Potion",
        "Mana Potion",
        "Attack Buff",
        "Speed Buff",
        "Shield"};

public:
    UserData() : username(""), diamonds(0) // constructor
    {
        filename = "user_data.txt";
        for (int i = 0; i < 4; i++)
        {
            pets[i] = "";
        }
        for (int i = 0; i < 5; i++)
        {
            itemQuantities[i] = 0;
        }
    }

    void initializeNewUser()
    {
        diamonds = 500;
        pets[0] = "Dragon";
        pets[1] = "Phoenix";
        pets[2] = "Griffin";
        pets[3] = "Unicorn";

        itemQuantities[0] = 3;
        itemQuantities[1] = 2;
    }

    bool userExists(const std::string &name)
    {
        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line))
        {
            if (line.find("Username: " + name) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    void loadUserData(const std::string &name)
    {
        std::ifstream file(filename);
        std::string line;
        bool found = false;

        while (std::getline(file, line))
        {
            if (line.find("Username: " + name) != std::string::npos)
            {
                found = true;
                username = name;

                std::getline(file, line);
                std::stringstream ss(line.substr(10));
                ss >> diamonds;

                for (int i = 0; i < 4; i++)
                {
                    std::getline(file, line);
                    pets[i] = line.substr(6);
                }

                for (int i = 0; i < 5; i++)
                {
                    std::getline(file, line);
                    std::stringstream ss(line.substr(8));
                    ss >> itemQuantities[i];
                }
                break;
            }
        }

        if (!found)
        {
            username = name;
            initializeNewUser();
            saveUserData();
        }
    }

    void saveUserData()
    {
        if (userExists(username))
        {
            updateUserData();
            return;
        }

        std::ofstream file(filename, std::ios::app);
        file << "Username: " << username << "\n";
        file << "Diamonds: " << diamonds << "\n";
        for (int i = 0; i < 4; i++)
        {
            file << "Pet " << i << ": " << pets[i] << "\n";
        }
        for (int i = 0; i < 5; i++)
        {
            file << "Item " << i << ": " << itemQuantities[i] << "\n";
        }
        file << "----------------\n";
    }

    void updateUserData()
    {
        std::ifstream inFile(filename);
        std::ofstream outFile("temp.txt");
        std::string line;
        bool found = false;

        while (std::getline(inFile, line))
        {
            if (line.find("Username: " + username) != std::string::npos)
            {
                found = true;
                outFile << "Username: " << username << "\n";
                outFile << "Diamonds: " << diamonds << "\n";
                for (int i = 0; i < 4; i++)
                {
                    std::getline(inFile, line);
                    outFile << "Pet " << i << ": " << pets[i] << "\n";
                }
                for (int i = 0; i < 5; i++)
                {
                    std::getline(inFile, line);
                    outFile << "Item " << i << ": " << itemQuantities[i] << "\n";
                }
                std::getline(inFile, line);
            }
            else
            {
                outFile << line << "\n";
            }
        }

        inFile.close();
        outFile.close();

        remove(filename.c_str());
        rename("temp.txt", filename.c_str());
    }

    void addDiamonds(int amount)
    {
        diamonds += amount;
        if (diamonds < 0)
            diamonds = 0;
    }
    void setItemQuantity(int index, int quantity)
    {
        if (index >= 0 && index < 5)
        {
            itemQuantities[index] = quantity;
            if (itemQuantities[index] < 0)
                itemQuantities[index] = 0;
        }
    }
    void addItemQuantity(int index, int amount)
    {
        if (index >= 0 && index < 5)
        {
            itemQuantities[index] += amount;
            if (itemQuantities[index] < 0)
                itemQuantities[index] = 0;
        }
    }
                                             // getter
    int getItemQuantity(int index) const
    {
        if (index >= 0 && index < 5)
            return itemQuantities[index];
        return 0;
    }
    std::string getItemName(int index) const
    {
        if (index >= 0 && index < 5)
            return itemNames[index];
        return "";
    }

    int getDiamonds() const { return diamonds; }
    std::string getUsername() const { return username; }
};

// ------------ 2V2 BATTLE GAME CLASS ---------------- //

// This is a 2v2 pet battle game where players and enemies control pets that move, shoot abilities (fire/ice/lightning/magic), and have health bars.
// It tracks game time, collisions, and win/lose conditions. It uses Encapsulation keeping  data  private and aggregation
class Battle2v2Game
{
private:
    bool isActive;
    bool gameOver;
    bool playerWon;

    sf::RectangleShape window;
    sf::RectangleShape backgroundDim;
    sf::Text title;
    Button closeButton;
    sf::Font font;

    Pet *playerPets[2];
    Pet *enemyPets[2];
    sf::Texture playerTextures[2];
    sf::Sprite playerSprites[2];
    sf::Texture enemyTextures[2];
    sf::Sprite enemySprites[2];

    float playerSpeed;
    float enemySpeed;
    sf::Vector2f playerVelocities[2];
    sf::Vector2f enemyVelocities[2];
    bool keys[8];

    struct Ability
    {
        sf::Sprite projectile;
        sf::Vector2f velocity;
        float cooldown;
        float currentCooldown;
        int petIndex;
        int damage;

                                                     // Overload == operator for ability comparison
        bool operator==(const Ability &other) const
        {
            return projectile.getPosition() == other.projectile.getPosition() &&
                   velocity == other.velocity &&
                   damage == other.damage;
        }
    };

    static const int MAX_ABILITIES = 100;
    Ability playerAbilities[MAX_ABILITIES];
    Ability enemyAbilities[MAX_ABILITIES];
    int playerAbilityCount;
    int enemyAbilityCount;
    sf::Clock abilityClock;

    int playerHealth[2];
    int enemyHealth[2];
    int playerMaxHealth[2];
    int enemyMaxHealth[2];
    sf::Text playerHealthText[2];
    sf::Text enemyHealthText[2];
    sf::RectangleShape playerHealthBar[2];
    sf::RectangleShape playerHealthBarBackground[2];
    sf::RectangleShape enemyHealthBar[2];
    sf::RectangleShape enemyHealthBarBackground[2];

    sf::Clock gameClock;
    sf::Clock powerDecreaseClock;
    sf::Text timerText;
    int gameDuration;

    sf::Texture fireTexture;
    sf::Texture iceTexture;
    sf::Texture lightningTexture;
    sf::Texture magicTexture;

    sf::FloatRect arenaBounds;
    sf::Vector2f arenaCenter;

    void setupAbilities()
    {
        playerAbilityCount = 0;
        enemyAbilityCount = 0;

        if (!fireTexture.loadFromFile("fire1.png"))
        {
            sf::Image placeholder;
            placeholder.create(50, 20, sf::Color(255, 150, 0));
            fireTexture.loadFromImage(placeholder);
        }
        if (!iceTexture.loadFromFile("ice1.png"))
        {
            sf::Image placeholder;
            placeholder.create(50, 20, sf::Color(100, 200, 255));
            iceTexture.loadFromImage(placeholder);
        }
        if (!lightningTexture.loadFromFile("lightning.png"))
        {
            sf::Image placeholder;
            placeholder.create(50, 20, sf::Color(255, 255, 100));
            lightningTexture.loadFromImage(placeholder);
        }
        if (!magicTexture.loadFromFile("magic.png"))
        {
            sf::Image placeholder;
            placeholder.create(50, 20, sf::Color(200, 100, 255));
            magicTexture.loadFromImage(placeholder);
        }
    }

    void resetPositions()
    {
        playerSprites[0].setPosition(arenaCenter.x - 400, arenaCenter.y - 100);
        playerSprites[1].setPosition(arenaCenter.x - 400, arenaCenter.y + 100);

        enemySprites[0].setPosition(arenaCenter.x + 400, arenaCenter.y - 100);
        enemySprites[1].setPosition(arenaCenter.x + 400, arenaCenter.y + 100);

        for (int i = 0; i < 2; i++)
        {
            playerVelocities[i] = sf::Vector2f(0, 0);
            enemyVelocities[i] = sf::Vector2f(0, 0);
        }

        playerAbilityCount = 0;
        enemyAbilityCount = 0;
    }

    bool checkCollision(const sf::Sprite &sprite1, const sf::Sprite &sprite2)
    {
        return sprite1.getGlobalBounds().intersects(sprite2.getGlobalBounds());
    }

    void decreasePowerOverTime()
    {
        if (powerDecreaseClock.getElapsedTime().asSeconds() > 5.0f) 
        {
            for (int i = 0; i < 2; i++)
            {
                if (playerHealth[i] > 0)
                {
                    int decreaseAmount = getPowerDecreaseRate(playerPets[i]->getType());
                    updateHealthBars();
                }

                if (enemyHealth[i] > 0)
                {
                    int decreaseAmount = getPowerDecreaseRate(enemyPets[i]->getType());
                    updateHealthBars();
                }
            }
            powerDecreaseClock.restart();
        }
    }

    int getPowerDecreaseRate(const std::string &petType) const
    {
        if (petType == "Dragon")
            return 8;
        if (petType == "Griffin")
            return 7;
        if (petType == "Phoenix")
            return 6;
        if (petType == "Unicorn")
            return 5;
        return 4; 
    }

    void updateHealthBars()
    {
        for (int i = 0; i < 2; i++)
        {
            if (playerHealth[i] > 0)
            {
                playerHealthText[i].setString(playerPets[i]->getName() + ": " +
                                              std::to_string(playerHealth[i]) + "/" + std::to_string(playerMaxHealth[i]));

                float healthPercentage = playerHealth[i] / static_cast<float>(playerMaxHealth[i]);
                playerHealthBar[i].setSize(sf::Vector2f(200 * healthPercentage, 20));
            }

            if (enemyHealth[i] > 0)
            {
                enemyHealthText[i].setString(enemyPets[i]->getName() + ": " +
                                             std::to_string(enemyHealth[i]) + "/" + std::to_string(enemyMaxHealth[i]));

                float healthPercentage = enemyHealth[i] / static_cast<float>(enemyMaxHealth[i]);
                enemyHealthBar[i].setSize(sf::Vector2f(200 * healthPercentage, 20));
            }
        }
    }

    void handleCollisions()
    {
        for (int i = 0; i < playerAbilityCount;)
        {
            bool hit = false;
            for (int j = 0; j < 2; j++)
            {
                if (enemyHealth[j] > 0 && checkCollision(playerAbilities[i].projectile, enemySprites[j]))
                {
                    enemyHealth[j] -= playerAbilities[i].damage;
                    enemyHealthText[j].setString(enemyPets[j]->getName() + ": " +
                                                 std::to_string(enemyHealth[j]) + "/" + std::to_string(enemyMaxHealth[j]));

                    enemyHealthBar[j].setSize(sf::Vector2f(
                        200 * (enemyHealth[j] / static_cast<float>(enemyMaxHealth[j])),
                        20));

                    if (enemyHealth[j] <= 0)
                    {
                        enemyHealth[j] = 0;
                        enemySprites[j].setColor(sf::Color(100, 100, 100));
                    }

                    hit = true;
                    break;
                }
            }

            if (hit)
            {
                playerAbilities[i] = playerAbilities[playerAbilityCount - 1];
                playerAbilityCount--;
            }
            else
            {
                i++;
            }
        }

        for (int i = 0; i < enemyAbilityCount;)
        {
            bool hit = false;
            for (int j = 0; j < 2; j++)
            {
                if (playerHealth[j] > 0 && checkCollision(enemyAbilities[i].projectile, playerSprites[j]))
                {
                    playerHealth[j] -= enemyAbilities[i].damage;
                    playerHealthText[j].setString(playerPets[j]->getName() + ": " +
                                                  std::to_string(playerHealth[j]) + "/" + std::to_string(playerMaxHealth[j]));

                    playerHealthBar[j].setSize(sf::Vector2f(
                        200 * (playerHealth[j] / static_cast<float>(playerMaxHealth[j])),
                        20));

                    if (playerHealth[j] <= 0)
                    {
                        playerHealth[j] = 0;
                        playerSprites[j].setColor(sf::Color(100, 100, 100));
                    }

                    hit = true;
                    break;
                }
            }

            if (hit)
            {
                enemyAbilities[i] = enemyAbilities[enemyAbilityCount - 1];
                enemyAbilityCount--;
            }
            else
            {
                i++;
            }
        }

        bool allPlayerPetsDefeated = true;
        bool allEnemyPetsDefeated = true;

        for (int i = 0; i < 2; i++)
        {
            if (playerHealth[i] > 0)
                allPlayerPetsDefeated = false;
            if (enemyHealth[i] > 0)
                allEnemyPetsDefeated = false;
        }

        if (allPlayerPetsDefeated || allEnemyPetsDefeated)
        {
            gameOver = true;
            playerWon = allEnemyPetsDefeated;
            if (playerWon)
            {
                for (int i = 0; i < 2; i++)
                {
                    if (playerHealth[i] > 0)
                    {
                        int xp = 50 + (rand() % 50) + (enemyMaxHealth[0] + enemyMaxHealth[1]) / 20;
                        playerPets[i]->gainExperience(xp);
                    }
                }
            }
        }
    }

    void removeWhiteBackground(sf::Image &image)
    {
        for (unsigned int y = 0; y < image.getSize().y; ++y)
        {
            for (unsigned int x = 0; x < image.getSize().x; ++x)
            {
                sf::Color pixel = image.getPixel(x, y);
                if (pixel.r > 200 && pixel.g > 200 && pixel.b > 200)
                {
                    pixel.a = 0;
                    image.setPixel(x, y, pixel);
                }
            }
        }
    }

    void addPlayerProjectile(int petIndex)
    {
        if (playerAbilityCount < MAX_ABILITIES && abilityClock.getElapsedTime().asSeconds() > 0.5f)
        {
            playerAbilities[playerAbilityCount].petIndex = petIndex;

            if (playerPets[petIndex]->getType() == "Fire")
            {
                playerAbilities[playerAbilityCount].projectile.setTexture(fireTexture);
                playerAbilities[playerAbilityCount].damage = playerPets[petIndex]->getAttack() / 2;
            }
            else if (playerPets[petIndex]->getType() == "Ice")
            {
                playerAbilities[playerAbilityCount].projectile.setTexture(iceTexture);
                playerAbilities[playerAbilityCount].damage = playerPets[petIndex]->getAttack() / 3;
            }
            else if (playerPets[petIndex]->getType() == "Electric")
            {
                playerAbilities[playerAbilityCount].projectile.setTexture(lightningTexture);
                playerAbilities[playerAbilityCount].damage = playerPets[petIndex]->getAttack() / 2;
            }
            else
            {
                playerAbilities[playerAbilityCount].projectile.setTexture(magicTexture);
                playerAbilities[playerAbilityCount].damage = playerPets[petIndex]->getAttack() / 3;
            }

            playerAbilities[playerAbilityCount].projectile.setScale(0.4f, 0.4f);

            sf::Vector2f targetPos;
            float minDistance = 9999;
            for (int i = 0; i < 2; i++)
            {
                if (enemyHealth[i] > 0)
                {
                    float dist = sqrt(pow(enemySprites[i].getPosition().x - playerSprites[petIndex].getPosition().x, 2) +
                                      pow(enemySprites[i].getPosition().y - playerSprites[petIndex].getPosition().y, 2));
                    if (dist < minDistance)
                    {
                        minDistance = dist;
                        targetPos = enemySprites[i].getPosition();
                    }
                }
            }

            if (minDistance == 9999)
            {
                playerAbilities[playerAbilityCount].velocity = sf::Vector2f(15.0f, 0);
            }
            else
            {
                sf::Vector2f direction = targetPos - playerSprites[petIndex].getPosition();
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length > 0)
                {
                    direction /= length;
                    playerAbilities[playerAbilityCount].velocity = direction * 15.0f;
                }
                else
                {
                    playerAbilities[playerAbilityCount].velocity = sf::Vector2f(15.0f, 0);
                }
            }

            playerAbilities[playerAbilityCount].cooldown = 0.5f;

            playerAbilities[playerAbilityCount].projectile.setPosition(
                playerSprites[petIndex].getPosition().x + playerSprites[petIndex].getGlobalBounds().width / 2,
                playerSprites[petIndex].getPosition().y + playerSprites[petIndex].getGlobalBounds().height / 2);

            playerAbilityCount++;
            abilityClock.restart();
        }
    }

    void addEnemyProjectile(int petIndex)
    {
        if (enemyAbilityCount < MAX_ABILITIES && rand() % 100 < 3)
        {
            enemyAbilities[enemyAbilityCount].petIndex = petIndex;

            if (enemyPets[petIndex]->getType() == "Fire")
            {
                enemyAbilities[enemyAbilityCount].projectile.setTexture(fireTexture);
                enemyAbilities[enemyAbilityCount].damage = enemyPets[petIndex]->getAttack() / 2;
            }
            else if (enemyPets[petIndex]->getType() == "Ice")
            {
                enemyAbilities[enemyAbilityCount].projectile.setTexture(iceTexture);
                enemyAbilities[enemyAbilityCount].damage = enemyPets[petIndex]->getAttack() / 3;
            }
            else if (enemyPets[petIndex]->getType() == "Electric")
            {
                enemyAbilities[enemyAbilityCount].projectile.setTexture(lightningTexture);
                enemyAbilities[enemyAbilityCount].damage = enemyPets[petIndex]->getAttack() / 2;
            }
            else
            {
                enemyAbilities[enemyAbilityCount].projectile.setTexture(magicTexture);
                enemyAbilities[enemyAbilityCount].damage = enemyPets[petIndex]->getAttack() / 3;
            }

            enemyAbilities[enemyAbilityCount].projectile.setScale(0.4f, 0.4f);

            sf::Vector2f targetPos;
            float minDistance = 9999;
            for (int i = 0; i < 2; i++)
            {
                if (playerHealth[i] > 0)
                {
                    float dist = sqrt(pow(playerSprites[i].getPosition().x - enemySprites[petIndex].getPosition().x, 2) +
                                      pow(playerSprites[i].getPosition().y - enemySprites[petIndex].getPosition().y, 2));
                    if (dist < minDistance)
                    {
                        minDistance = dist;
                        targetPos = playerSprites[i].getPosition();
                    }
                }
            }

            if (minDistance == 9999)
            {
                enemyAbilities[enemyAbilityCount].velocity = sf::Vector2f(-12.0f, 0);
            }
            else
            {
                sf::Vector2f direction = targetPos - enemySprites[petIndex].getPosition();
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length > 0)
                {
                    direction /= length;
                    enemyAbilities[enemyAbilityCount].velocity = direction * 12.0f;
                }
                else
                {
                    enemyAbilities[enemyAbilityCount].velocity = sf::Vector2f(-12.0f, 0);
                }
            }

            enemyAbilities[enemyAbilityCount].cooldown = 1.0f;

            enemyAbilities[enemyAbilityCount].projectile.setPosition(
                enemySprites[petIndex].getPosition().x,
                enemySprites[petIndex].getPosition().y + enemySprites[petIndex].getGlobalBounds().height / 2);

            enemyAbilityCount++;
        }
    }

public:
    Battle2v2Game() : isActive(false), gameOver(false), playerWon(false),
                      gameDuration(180), playerSpeed(5.0f), enemySpeed(3.0f),
                      playerAbilityCount(0), enemyAbilityCount(0)
    {
        for (int i = 0; i < 8; i++)
        {
            keys[i] = false;
        }
        for (int i = 0; i < 2; i++)
        {
            playerVelocities[i] = sf::Vector2f(0, 0);
            enemyVelocities[i] = sf::Vector2f(0, 0);
            playerHealth[i] = 0;
            enemyHealth[i] = 0;
            playerMaxHealth[i] = 0;
            enemyMaxHealth[i] = 0;
        }
    }

    void setup(const sf::Font &gameFont, Pet *player1, Pet *player2, Pet *enemy1, Pet *enemy2)
    {
        font = gameFont;
        playerPets[0] = player1;
        playerPets[1] = player2;
        enemyPets[0] = enemy1;
        enemyPets[1] = enemy2;

        for (int i = 0; i < 8; i++)
        {
            keys[i] = false;
        }

        for (int i = 0; i < 2; i++)
        {
            playerVelocities[i] = sf::Vector2f(0, 0);
            enemyVelocities[i] = sf::Vector2f(0, 0);
        }

        backgroundDim.setFillColor(sf::Color(0, 0, 0, 180));
        window.setSize(sf::Vector2f(1200, 700));
        window.setFillColor(sf::Color(40, 40, 50, 240));
        window.setOutlineThickness(4.f);
        window.setOutlineColor(sf::Color(255, 215, 0));

        arenaBounds = sf::FloatRect(100, 150, window.getSize().x - 200, window.getSize().y - 250); // Adjusted for better spacing
        arenaCenter = sf::Vector2f(window.getSize().x / 2, window.getSize().y / 2);

        title.setFont(font);
        title.setString("2 vs 2 BATTLE ARENA");
        title.setCharacterSize(42);
        title.setFillColor(sf::Color(255, 215, 0));
        title.setStyle(sf::Text::Bold);
        title.setOutlineThickness(2.f);
        title.setOutlineColor(sf::Color::Black);

        closeButton = Button(font, "CLOSE", 28,
                             sf::Color(255, 100, 100),
                             sf::Color(255, 150, 150),
                             sf::Color(200, 50, 50));

        for (int i = 0; i < 2; i++)
        {
            sf::Image playerImg;
            if (playerImg.loadFromFile(playerPets[i]->getTexturePath()))
            {
                removeWhiteBackground(playerImg);
                playerTextures[i].loadFromImage(playerImg);
                playerSprites[i].setTexture(playerTextures[i]);
            }
            else
            {
                sf::Image placeholder;
                placeholder.create(100, 100, sf::Color::Magenta);
                playerTextures[i].loadFromImage(placeholder);
                playerSprites[i].setTexture(playerTextures[i]);
            }
            playerSprites[i].setScale(0.8f, 0.8f);
            playerMaxHealth[i] = playerPets[i]->getHP();
            playerHealth[i] = playerMaxHealth[i];
        }

        for (int i = 0; i < 2; i++)
        {
            sf::Image enemyImg;
            if (enemyImg.loadFromFile(enemyPets[i]->getTexturePath()))
            {
                removeWhiteBackground(enemyImg);
                enemyTextures[i].loadFromImage(enemyImg);
                enemySprites[i].setTexture(enemyTextures[i]);
            }
            else
            {
                sf::Image placeholder;
                placeholder.create(100, 100, sf::Color::Cyan);
                enemyTextures[i].loadFromImage(placeholder);
                enemySprites[i].setTexture(enemyTextures[i]);
            }
            enemySprites[i].setScale(0.8f, 0.8f);
            enemyMaxHealth[i] = enemyPets[i]->getHP();
            enemyHealth[i] = enemyMaxHealth[i];
        }

        for (int i = 0; i < 2; i++)
        {
            playerHealthText[i].setFont(font);
            playerHealthText[i].setString(playerPets[i]->getName() + ": " + std::to_string(playerHealth[i]) + "/" + std::to_string(playerMaxHealth[i]));
            playerHealthText[i].setCharacterSize(24);
            playerHealthText[i].setFillColor(sf::Color(255, 150, 100));
            playerHealthText[i].setOutlineThickness(1.f);
            playerHealthText[i].setOutlineColor(sf::Color::Black);

            enemyHealthText[i].setFont(font);
            enemyHealthText[i].setString(enemyPets[i]->getName() + ": " + std::to_string(enemyHealth[i]) + "/" + std::to_string(enemyMaxHealth[i]));
            enemyHealthText[i].setCharacterSize(24);
            enemyHealthText[i].setFillColor(sf::Color(100, 150, 255));
            enemyHealthText[i].setOutlineThickness(1.f);
            enemyHealthText[i].setOutlineColor(sf::Color::Black);

            playerHealthBarBackground[i].setSize(sf::Vector2f(200, 20));
            playerHealthBarBackground[i].setFillColor(sf::Color(50, 50, 50));
            playerHealthBarBackground[i].setOutlineThickness(2.f);
            playerHealthBarBackground[i].setOutlineColor(sf::Color::Black);

            playerHealthBar[i].setSize(sf::Vector2f(200, 20));
            playerHealthBar[i].setFillColor(sf::Color(255, 50, 50));

            enemyHealthBarBackground[i].setSize(sf::Vector2f(200, 20));
            enemyHealthBarBackground[i].setFillColor(sf::Color(50, 50, 50));
            enemyHealthBarBackground[i].setOutlineThickness(2.f);
            enemyHealthBarBackground[i].setOutlineColor(sf::Color::Black);

            enemyHealthBar[i].setSize(sf::Vector2f(200, 20));
            enemyHealthBar[i].setFillColor(sf::Color(50, 50, 255));
        }

        timerText.setFont(font);
        timerText.setString("TIME: 180");
        timerText.setCharacterSize(28);
        timerText.setFillColor(sf::Color(255, 215, 0));
        timerText.setOutlineThickness(1.f);
        timerText.setOutlineColor(sf::Color::Black);

        setupAbilities();
        resetPositions();
    }

    void open()
    {
        isActive = true;
        gameOver = false;
        playerWon = false;

        for (int i = 0; i < 2; i++)
        {
            playerMaxHealth[i] = playerPets[i]->getHP();
            playerHealth[i] = playerMaxHealth[i];
            enemyMaxHealth[i] = enemyPets[i]->getHP();
            enemyHealth[i] = enemyMaxHealth[i];
            playerHealthText[i].setString(playerPets[i]->getName() + ": " + std::to_string(playerHealth[i]) + "/" + std::to_string(playerMaxHealth[i]));
            enemyHealthText[i].setString(enemyPets[i]->getName() + ": " + std::to_string(enemyHealth[i]) + "/" + std::to_string(enemyMaxHealth[i]));
            playerHealthBar[i].setSize(sf::Vector2f(200, 20));
            enemyHealthBar[i].setSize(sf::Vector2f(200, 20));
            playerSprites[i].setColor(sf::Color::White);
            enemySprites[i].setColor(sf::Color::White);
        }

        gameClock.restart();
        powerDecreaseClock.restart();
        resetPositions();
    }

    void handleInput(const sf::Event &event, const sf::Vector2f &mousePos)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
            case sf::Keyboard::W:
                keys[0] = true;
                break;
            case sf::Keyboard::A:
                keys[1] = true;
                break;
            case sf::Keyboard::S:
                keys[2] = true;
                break;
            case sf::Keyboard::D:
                keys[3] = true;
                break;
            case sf::Keyboard::Space:
                if (playerHealth[0] > 0)
                {
                    addPlayerProjectile(0);
                }
                break;
            }

            switch (event.key.code)
            {
            case sf::Keyboard::I:
                keys[4] = true;
                break;
            case sf::Keyboard::J:
                keys[5] = true;
                break;
            case sf::Keyboard::K:
                keys[6] = true;
                break;
            case sf::Keyboard::L:
                keys[7] = true;
                break;
            case sf::Keyboard::M:
                if (playerHealth[1] > 0)
                {
                    addPlayerProjectile(1);
                }
                break;
            }
        }
        else if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
            case sf::Keyboard::W:
                keys[0] = false;
                break;
            case sf::Keyboard::A:
                keys[1] = false;
                break;
            case sf::Keyboard::S:
                keys[2] = false;
                break;
            case sf::Keyboard::D:
                keys[3] = false;
                break;
            }

            switch (event.key.code)
            {
            case sf::Keyboard::I:
                keys[4] = false;
                break;
            case sf::Keyboard::J:
                keys[5] = false;
                break;
            case sf::Keyboard::K:
                keys[6] = false;
                break;
            case sf::Keyboard::L:
                keys[7] = false;
                break;
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed &&
                 event.mouseButton.button == sf::Mouse::Left)
        {
            if (closeButton.contains(mousePos))
            {
                close();
            }
        }
    }

    void update()
    {
        if (gameOver)
            return;

        decreasePowerOverTime();

        playerVelocities[0].x = (keys[3] - keys[1]) * playerSpeed; // D - A
        playerVelocities[0].y = (keys[2] - keys[0]) * playerSpeed; // S - W

        playerVelocities[1].x = (keys[7] - keys[5]) * playerSpeed; // L - J
        playerVelocities[1].y = (keys[6] - keys[4]) * playerSpeed; // K - I

        for (int i = 0; i < 2; i++)
        {
            if (playerVelocities[i].x != 0 && playerVelocities[i].y != 0)
            {
                playerVelocities[i] *= 0.7071f;
            }
        }

        for (int i = 0; i < 2; i++)
        {
            if (playerHealth[i] > 0)
            {
                playerSprites[i].move(playerVelocities[i]);

                sf::FloatRect spriteBounds = playerSprites[i].getGlobalBounds();

                if (spriteBounds.left < arenaBounds.left)
                    playerSprites[i].setPosition(arenaBounds.left, spriteBounds.top);
                if (spriteBounds.left + spriteBounds.width > arenaBounds.left + arenaBounds.width)
                    playerSprites[i].setPosition(arenaBounds.left + arenaBounds.width - spriteBounds.width, spriteBounds.top);
                if (spriteBounds.top < arenaBounds.top)
                    playerSprites[i].setPosition(spriteBounds.left, arenaBounds.top);
                if (spriteBounds.top + spriteBounds.height > arenaBounds.top + arenaBounds.height)
                    playerSprites[i].setPosition(spriteBounds.left, arenaBounds.top + arenaBounds.height - spriteBounds.height);
            }
        }

        // Enemy AI movement
        for (int i = 0; i < 2; i++)
        {
            if (enemyHealth[i] > 0)
            {
                float minDistance = 9999;
                int targetIndex = -1;

                for (int j = 0; j < 2; j++)
                {
                    if (playerHealth[j] > 0)
                    {
                        float dx = playerSprites[j].getPosition().x - enemySprites[i].getPosition().x;
                        float dy = playerSprites[j].getPosition().y - enemySprites[i].getPosition().y;
                        float distance = sqrt(dx * dx + dy * dy);

                        if (distance < minDistance)
                        {
                            minDistance = distance;
                            targetIndex = j;
                        }
                    }
                }

                if (targetIndex >= 0)
                {
                    sf::Vector2f direction = playerSprites[targetIndex].getPosition() - enemySprites[i].getPosition();
                    float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                    if (length > 0)
                    {
                        direction /= length;

                        direction.x += ((rand() % 100) - 50) * 0.01f;
                        direction.y += ((rand() % 100) - 50) * 0.01f;

                        length = sqrt(direction.x * direction.x + direction.y * direction.y);
                        if (length > 0)
                        {
                            direction /= length;
                        }

                        enemyVelocities[i] = direction * enemySpeed;
                    }
                }
                else
                {
                    enemyVelocities[i].x = ((rand() % 100) - 50) * 0.02f;
                    enemyVelocities[i].y = ((rand() % 100) - 50) * 0.02f;
                }

                enemySprites[i].move(enemyVelocities[i]);

                sf::FloatRect spriteBounds = enemySprites[i].getGlobalBounds();

                if (spriteBounds.left < arenaBounds.left)
                    enemySprites[i].setPosition(arenaBounds.left, spriteBounds.top);
                if (spriteBounds.left + spriteBounds.width > arenaBounds.left + arenaBounds.width)
                    enemySprites[i].setPosition(arenaBounds.left + arenaBounds.width - spriteBounds.width, spriteBounds.top);
                if (spriteBounds.top < arenaBounds.top)
                    enemySprites[i].setPosition(spriteBounds.left, arenaBounds.top);
                if (spriteBounds.top + spriteBounds.height > arenaBounds.top + arenaBounds.height)
                    enemySprites[i].setPosition(spriteBounds.left, arenaBounds.top + arenaBounds.height - spriteBounds.height);
            }
        }

        // Enemy AI shooting
        for (int i = 0; i < 2; i++)
        {
            if (enemyHealth[i] > 0)
            {
                addEnemyProjectile(i);
            }
        }

        for (int i = 0; i < playerAbilityCount; i++)
        {
            playerAbilities[i].projectile.move(playerAbilities[i].velocity);

            if (playerAbilities[i].projectile.getPosition().x > arenaBounds.left + arenaBounds.width ||
                playerAbilities[i].projectile.getPosition().y < arenaBounds.top ||
                playerAbilities[i].projectile.getPosition().y > arenaBounds.top + arenaBounds.height)
            {
                playerAbilities[i] = playerAbilities[playerAbilityCount - 1];
                playerAbilityCount--;
                i--;
            }
        }

        for (int i = 0; i < enemyAbilityCount; i++)
        {
            enemyAbilities[i].projectile.move(enemyAbilities[i].velocity);

            if (enemyAbilities[i].projectile.getPosition().x + enemyAbilities[i].projectile.getGlobalBounds().width < arenaBounds.left ||
                enemyAbilities[i].projectile.getPosition().y < arenaBounds.top ||
                enemyAbilities[i].projectile.getPosition().y > arenaBounds.top + arenaBounds.height)
            {
                enemyAbilities[i] = enemyAbilities[enemyAbilityCount - 1];
                enemyAbilityCount--;
                i--;
            }
        }

        handleCollisions();

        int remainingTime = gameDuration - gameClock.getElapsedTime().asSeconds();
        if (remainingTime < 0)
            remainingTime = 0;
        timerText.setString("TIME: " + std::to_string(remainingTime));

        if (remainingTime <= 0)
        {
            gameOver = true;
            int playerTotal = playerHealth[0] + playerHealth[1];
            int enemyTotal = enemyHealth[0] + enemyHealth[1];
            playerWon = (playerTotal > enemyTotal);

            if (playerWon)
            {
                for (int i = 0; i < 2; i++)
                {
                    if (playerHealth[i] > 0)
                    {
                        int xp = 30 + (rand() % 30) + enemyTotal / 20;
                        playerPets[i]->gainExperience(xp);
                    }
                }
            }
        }
    }

    void draw(sf::RenderWindow &targetWindow)
    {
        if (!isActive)
            return;

        sf::Vector2u winSize = targetWindow.getSize();
        backgroundDim.setSize(sf::Vector2f(winSize.x, winSize.y));
        targetWindow.draw(backgroundDim);

        sf::FloatRect windowBounds = window.getLocalBounds();
        window.setPosition((winSize.x - windowBounds.width) / 2,
                           (winSize.y - windowBounds.height) / 2);

        title.setPosition(
            window.getPosition().x + (windowBounds.width - title.getLocalBounds().width) / 2,
            window.getPosition().y + 15);

        timerText.setPosition(
            window.getPosition().x + (windowBounds.width - timerText.getLocalBounds().width) / 2,
            window.getPosition().y + 70); 

        closeButton.setPosition(
            window.getPosition().x + windowBounds.width - closeButton.getBounds().width - 20,
            window.getPosition().y + 15);

        for (int i = 0; i < 2; i++)
        {
            playerHealthText[i].setPosition(
                window.getPosition().x + 20,
                window.getPosition().y + 120 + i * 250); 

            enemyHealthText[i].setPosition(
                window.getPosition().x + windowBounds.width - enemyHealthText[i].getLocalBounds().width - 20,
                window.getPosition().y + 120 + i * 250); 

            playerHealthBarBackground[i].setPosition(
                window.getPosition().x + 20,
                window.getPosition().y + 150 + i * 250); 

            playerHealthBar[i].setPosition(
                window.getPosition().x + 20,
                window.getPosition().y + 150 + i * 250); 

            enemyHealthBarBackground[i].setPosition(
                window.getPosition().x + windowBounds.width - 220,
                window.getPosition().y + 150 + i * 250); 

            enemyHealthBar[i].setPosition(
                window.getPosition().x + windowBounds.width - 220,
                window.getPosition().y + 150 + i * 250); 
        }

        targetWindow.draw(window);
        targetWindow.draw(title);
        targetWindow.draw(timerText);

        if (!gameOver)
        {
            for (int i = 0; i < 2; i++)
            {
                targetWindow.draw(playerHealthBarBackground[i]);
                targetWindow.draw(enemyHealthBarBackground[i]);
            }

            for (int i = 0; i < 2; i++)
            {
                targetWindow.draw(playerHealthBar[i]);
                targetWindow.draw(enemyHealthBar[i]);
            }

            for (int i = 0; i < 2; i++)
            {
                targetWindow.draw(playerHealthText[i]);
                targetWindow.draw(enemyHealthText[i]);
            }

            for (int i = 0; i < 2; i++)
            {
                targetWindow.draw(playerSprites[i]);
                targetWindow.draw(enemySprites[i]);
            }

            for (int i = 0; i < playerAbilityCount; i++)
            {
                targetWindow.draw(playerAbilities[i].projectile);
            }

            for (int i = 0; i < enemyAbilityCount; i++)
            {
                targetWindow.draw(enemyAbilities[i].projectile);
            }
        }
        else
        {
            sf::Text gameOverText;
            gameOverText.setFont(font);
            gameOverText.setString(playerWon ? "VICTORY!" : "DEFEAT!");
            gameOverText.setCharacterSize(72);
            gameOverText.setFillColor(playerWon ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
            gameOverText.setStyle(sf::Text::Bold);
            gameOverText.setOutlineThickness(2.f);
            gameOverText.setOutlineColor(sf::Color::Black);
            gameOverText.setPosition(
                window.getPosition().x + (windowBounds.width - gameOverText.getLocalBounds().width) / 2,
                window.getPosition().y + windowBounds.height / 2 - 50);

            sf::Text scoreText;
            scoreText.setFont(font);

            int playerTotal = playerHealth[0] + playerHealth[1];
            int enemyTotal = enemyHealth[0] + enemyHealth[1];

            scoreText.setString("Your Pets: " + std::to_string(playerTotal) +
                                "  |  Enemy Pets: " + std::to_string(enemyTotal));
            scoreText.setCharacterSize(36);
            scoreText.setFillColor(sf::Color::White);
            scoreText.setOutlineThickness(1.f);
            scoreText.setOutlineColor(sf::Color::Black);
            scoreText.setPosition(
                window.getPosition().x + (windowBounds.width - scoreText.getLocalBounds().width) / 2,
                window.getPosition().y + windowBounds.height / 2 + 20);

            targetWindow.draw(gameOverText);
            targetWindow.draw(scoreText);
        }

        closeButton.draw(targetWindow);
    }

    bool isOpen() const { return isActive; }
    void close() { isActive = false; }
};
//---------------- 1V1 BATTLEGAME CLASS ----------------//

// 1v1 pet battle game with movement, abilities, and dynamic health bars.
//  It Uses encapsulation  and polymorphism like pets share traits but attack differently based on type.
class BattleGame
{
private:
    bool isActive;
    bool gameOver;
    bool playerWon;

    sf::RectangleShape window;
    sf::RectangleShape backgroundDim;
    sf::Text title;
    Button closeButton;
    sf::Font font;

    Pet *playerPet;
    Pet *enemyPet;
    sf::Texture playerTexture;
    sf::Sprite playerSprite;
    sf::Texture enemyTexture;
    sf::Sprite enemySprite;

    float playerSpeed;
    float enemySpeed;
    sf::Vector2f playerVelocity;
    sf::Vector2f enemyVelocity;
    bool keys[4]; // W, A, S, D

    struct Ability
    {
        sf::Sprite projectile;
        sf::Vector2f velocity;
        float cooldown;
        float currentCooldown;
        sf::Sound sound;
    };

    static const int MAX_ABILITIES = 50;
    Ability playerAbilities[MAX_ABILITIES];
    Ability enemyAbilities[MAX_ABILITIES];
    int playerAbilityCount;
    int enemyAbilityCount;
    sf::Clock abilityClock;

    static const int MAX_OBSTACLES = 10;
    sf::Sprite obstacles[MAX_OBSTACLES];
    int obstacleCount;
    sf::Texture obstacleTexture;
    sf::Clock obstacleSpawnClock;
    float obstacleSpawnInterval;
    float obstacleSpeed;

    int playerHealth;
    int enemyHealth;
    sf::Text playerHealthText;
    sf::Text enemyHealthText;
    sf::RectangleShape playerHealthBar;
    sf::RectangleShape enemyHealthBar;
    sf::RectangleShape playerHealthBarBack;
    sf::RectangleShape enemyHealthBarBack;
    sf::Clock gameClock;
    sf::Text timerText;
    int gameDuration;

    sf::SoundBuffer hitSoundBuffer;
    sf::Sound hitSound;
    sf::SoundBuffer winSoundBuffer;
    sf::Sound winSound;
    sf::SoundBuffer fireSoundBuffer;
    sf::Sound fireSound;

    sf::Texture fireTexture;
    sf::Texture iceTexture;

    void setupAbilities()
    {
        playerAbilityCount = 0;
        enemyAbilityCount = 0;

        std::string playerProjTexture;
        if (playerPet->getName() == "Dragon")
        {
            playerProjTexture = "fire1.png";
        }
        else if (playerPet->getName() == "Phoenix")
        {
            playerProjTexture = "ice1.png";
        }
        else if (playerPet->getName() == "Unicorn")
        {
            playerProjTexture = "magic.png";
        }
        else if (playerPet->getName() == "Griffin")
        {
            playerProjTexture = "lightning.png";
        }
        else
        {
            playerProjTexture = "default.png";
        }

        if (!fireTexture.loadFromFile(playerProjTexture))
        {
            sf::Image placeholder;
            if (playerPet->getName() == "Dragon")
            {
                placeholder.create(50, 20, sf::Color(255, 150, 0)); // Orange for fire
            }
            else if (playerPet->getName() == "Phoenix")
            {
                placeholder.create(50, 20, sf::Color(100, 200, 255)); // Blue for ice
            }
            else if (playerPet->getName() == "Unicorn")
            {
                placeholder.create(50, 20, sf::Color(200, 100, 255)); // Purple for magic
            }
            else if (playerPet->getName() == "Griffin")
            {
                placeholder.create(50, 20, sf::Color(255, 255, 100)); // Yellow for lightning
            }
            else
            {
                placeholder.create(50, 20, sf::Color::White); // Default
            }
            fireTexture.loadFromImage(placeholder);
        }

        if (playerAbilityCount < MAX_ABILITIES)
        {
            playerAbilities[playerAbilityCount].projectile.setTexture(fireTexture);
            playerAbilities[playerAbilityCount].projectile.setScale(0.4f, 0.4f);
            playerAbilities[playerAbilityCount].velocity = sf::Vector2f(15.0f, 0);
            playerAbilities[playerAbilityCount].cooldown = 0.5f;
            if (fireSoundBuffer.loadFromFile("fire.wav"))
            {
                playerAbilities[playerAbilityCount].sound.setBuffer(fireSoundBuffer);
            }
            playerAbilityCount++;
        }

        std::string enemyProjTexture;
        if (enemyPet->getName() == "Dragon")
        {
            enemyProjTexture = "fire1.png";
        }
        else if (enemyPet->getName() == "Phoenix")
        {
            enemyProjTexture = "ice1.png";
        }
        else if (enemyPet->getName() == "Unicorn")
        {
            enemyProjTexture = "magic.png";
        }
        else if (enemyPet->getName() == "Griffin")
        {
            enemyProjTexture = "lightning.png";
        }
        else
        {
            enemyProjTexture = "default.png";
        }

        if (!iceTexture.loadFromFile(enemyProjTexture))
        {
            sf::Image placeholder;
            if (enemyPet->getName() == "Dragon")
            {
                placeholder.create(50, 20, sf::Color(255, 150, 0)); // Orange for fire
            }
            else if (enemyPet->getName() == "Phoenix")
            {
                placeholder.create(50, 20, sf::Color(100, 200, 255)); // Blue for ice
            }
            else if (enemyPet->getName() == "Unicorn")
            {
                placeholder.create(50, 20, sf::Color(200, 100, 255)); // Purple for magic
            }
            else if (enemyPet->getName() == "Griffin")
            {
                placeholder.create(50, 20, sf::Color(255, 255, 100)); // Yellow for lightning
            }
            else
            {
                placeholder.create(50, 20, sf::Color::White); // Default
            }
            iceTexture.loadFromImage(placeholder);
        }

        if (enemyAbilityCount < MAX_ABILITIES)
        {
            enemyAbilities[enemyAbilityCount].projectile.setTexture(iceTexture);
            enemyAbilities[enemyAbilityCount].projectile.setScale(0.4f, 0.4f);
            enemyAbilities[enemyAbilityCount].velocity = sf::Vector2f(-12.0f, 0);
            enemyAbilities[enemyAbilityCount].cooldown = 1.0f;
            if (hitSoundBuffer.loadFromFile("hit.wav"))
            {
                enemyAbilities[enemyAbilityCount].sound.setBuffer(hitSoundBuffer);
            }
            enemyAbilityCount++;
        }
    }

    void setupObstacles()
    {
        obstacleCount = 0;
        obstacleSpawnInterval = 2.5f;
        obstacleSpeed = 3.5f;

        if (!obstacleTexture.loadFromFile("obstacle1.png"))
        {
            sf::Image placeholder;
            placeholder.create(60, 60, sf::Color(150, 75, 0));
            obstacleTexture.loadFromImage(placeholder);
        }
    }

    void spawnObstacle()
    {
        if (obstacleCount < MAX_OBSTACLES)
        {
            obstacles[obstacleCount].setTexture(obstacleTexture);
            float x = rand() % static_cast<int>(window.getSize().x - 100) + 50;
            obstacles[obstacleCount].setPosition(x, -50);
            obstacleCount++;
        }
    }

    void updateObstacles()
    {
        if (obstacleSpawnClock.getElapsedTime().asSeconds() > obstacleSpawnInterval)
        {
            spawnObstacle();
            obstacleSpawnClock.restart();
        }

        for (int i = 0; i < obstacleCount; i++)
        {
            obstacles[i].move(0, obstacleSpeed);
        }

        for (int i = 0; i < obstacleCount;)
        {
            if (obstacles[i].getPosition().y > window.getSize().y)
            {
                obstacles[i] = obstacles[obstacleCount - 1];
                obstacleCount--;
            }
            else
            {
                i++;
            }
        }
    }

    void resetPositions()
    {
        playerSprite.setPosition(150, 300);
        enemySprite.setPosition(800, 300);
        playerVelocity = sf::Vector2f(0, 0);
        enemyVelocity = sf::Vector2f(0, 0);

        playerAbilityCount = 0;
        enemyAbilityCount = 0;
        setupAbilities();

        obstacleCount = 0;
        obstacleSpawnClock.restart();
    }

    bool checkCollision(const sf::Sprite &sprite1, const sf::Sprite &sprite2)
    {
        return sprite1.getGlobalBounds().intersects(sprite2.getGlobalBounds());
    }

    void handleCollisions()
    {
        if (gameOver)
            return;

        for (int i = 0; i < obstacleCount; i++)
        {
            if (checkCollision(playerSprite, obstacles[i]))
            {
                sf::Vector2f pushDirection = playerSprite.getPosition() - obstacles[i].getPosition();
                float length = sqrt(pushDirection.x * pushDirection.x + pushDirection.y * pushDirection.y);
                if (length > 0)
                {
                    pushDirection /= length;
                    playerSprite.move(pushDirection * 5.0f);
                }

                playerHealth = std::max(0, playerHealth - 3);
                playerHealthText.setString(std::to_string(playerHealth));
                hitSound.play();

                playerHealthBar.setSize(sf::Vector2f(200 * (playerHealth / 100.f), 20));

                if (playerHealth <= 0)
                {
                    gameOver = true;
                    playerWon = false;
                    return;
                }
            }

            if (checkCollision(enemySprite, obstacles[i]))
            {
                sf::Vector2f pushDirection = enemySprite.getPosition() - obstacles[i].getPosition();
                float length = sqrt(pushDirection.x * pushDirection.x + pushDirection.y * pushDirection.y);
                if (length > 0)
                {
                    pushDirection /= length;
                    enemySprite.move(pushDirection * 5.0f);
                }

                enemyHealth = std::max(0, enemyHealth - 3);
                enemyHealthText.setString(std::to_string(enemyHealth));
                hitSound.play();

                enemyHealthBar.setSize(sf::Vector2f(200 * (enemyHealth / 100.f), 20));

                if (enemyHealth <= 0)
                {
                    gameOver = true;
                    playerWon = true;
                    winSound.play();
                    return;
                }
            }
        }
    }

    void removeWhiteBackground(sf::Image &image)
    {
        for (unsigned int y = 0; y < image.getSize().y; ++y)
        {
            for (unsigned int x = 0; x < image.getSize().x; ++x)
            {
                sf::Color pixel = image.getPixel(x, y);
                if (pixel.r > 200 && pixel.g > 200 && pixel.b > 200)
                {
                    pixel.a = 0;
                    image.setPixel(x, y, pixel);
                }
            }
        }
    }

    void loadPetTexture(Pet *pet, sf::Texture &texture, sf::Sprite &sprite)
    {
        sf::Image img;
        if (img.loadFromFile(pet->getTexturePath()))
        {
            removeWhiteBackground(img);
            if (pet->getName() == "Unicorn")
            {
                if (img.getSize().x < 64 || img.getSize().y < 64)
                {
                    sf::Image largerImg;
                    unsigned int newSize = std::max(img.getSize().x, img.getSize().y) * 2; 
                    largerImg.create(newSize, newSize, sf::Color::Transparent);

                    unsigned int offsetX = (newSize - img.getSize().x) / 2;
                    unsigned int offsetY = (newSize - img.getSize().y) / 2;

                    for (unsigned int y = 0; y < img.getSize().y; ++y)
                    {
                        for (unsigned int x = 0; x < img.getSize().x; ++x)
                        {
                            largerImg.setPixel(x + offsetX, y + offsetY, img.getPixel(x, y));
                        }
                    }
                    texture.loadFromImage(largerImg);
                }
                else
                {
                    texture.loadFromImage(img);
                }
            }
            else
            {
                texture.loadFromImage(img);
            }

            sprite.setTexture(texture);

            float baseSize = 100.0f; 
            float scaleX = baseSize / texture.getSize().x;
            float scaleY = baseSize / texture.getSize().y;
            sprite.setScale(scaleX * 0.8f, scaleY * 0.8f);
        }
        else
        {
            sf::Image placeholder;
            placeholder.create(100, 100,
                               pet == playerPet ? sf::Color::Magenta : sf::Color::Cyan);
            texture.loadFromImage(placeholder);
            sprite.setTexture(texture);
            sprite.setScale(0.8f, 0.8f);
        }
    }

public:
    BattleGame() : isActive(false), gameOver(false), playerWon(false),               // constructor
                   gameDuration(80), playerSpeed(5.0f), enemySpeed(3.5f),
                   playerHealth(100), enemyHealth(100),
                   playerAbilityCount(0), enemyAbilityCount(0),
                   obstacleCount(0)
    {
        for (int i = 0; i < 4; i++)
            keys[i] = false;
        playerVelocity = sf::Vector2f(0, 0);
    }

    void setup(const sf::Font &gameFont, Pet *player, Pet *enemy)
    {
        font = gameFont;
        playerPet = player;
        enemyPet = enemy;

        backgroundDim.setFillColor(sf::Color(0, 0, 0, 180));
        window.setSize(sf::Vector2f(1000, 600));
        window.setFillColor(sf::Color(40, 40, 50, 240));
        window.setOutlineThickness(4.f);
        window.setOutlineColor(sf::Color(255, 215, 0));

        title.setFont(font);
        title.setString("PET BATTLE ARENA");
        title.setCharacterSize(42);
        title.setFillColor(sf::Color(255, 215, 0));
        title.setStyle(sf::Text::Bold);
        title.setOutlineThickness(2.f);
        title.setOutlineColor(sf::Color::Black);

        closeButton = Button(font, "CLOSE", 28,
                             sf::Color(255, 100, 100),
                             sf::Color(255, 150, 150),
                             sf::Color(200, 50, 50));

        loadPetTexture(playerPet, playerTexture, playerSprite);
        loadPetTexture(enemyPet, enemyTexture, enemySprite);

        playerHealthText.setFont(font);
        playerHealthText.setString("100");
        playerHealthText.setCharacterSize(28);
        playerHealthText.setFillColor(sf::Color::White);
        playerHealthText.setOutlineThickness(1.f);
        playerHealthText.setOutlineColor(sf::Color::Black);

        enemyHealthText.setFont(font);
        enemyHealthText.setString("100");
        enemyHealthText.setCharacterSize(28);
        enemyHealthText.setFillColor(sf::Color::White);
        enemyHealthText.setOutlineThickness(1.f);
        enemyHealthText.setOutlineColor(sf::Color::Black);

        playerHealthBarBack.setSize(sf::Vector2f(200, 20));
        playerHealthBarBack.setFillColor(sf::Color(50, 50, 50));
        playerHealthBarBack.setOutlineThickness(2.f);
        playerHealthBarBack.setOutlineColor(sf::Color::Black);

        enemyHealthBarBack.setSize(sf::Vector2f(200, 20));
        enemyHealthBarBack.setFillColor(sf::Color(50, 50, 50));
        enemyHealthBarBack.setOutlineThickness(2.f);
        enemyHealthBarBack.setOutlineColor(sf::Color::Black);

        playerHealthBar.setSize(sf::Vector2f(200, 20));
        playerHealthBar.setFillColor(sf::Color(255, 50, 50));

        enemyHealthBar.setSize(sf::Vector2f(200, 20));
        enemyHealthBar.setFillColor(sf::Color(50, 50, 255));

        timerText.setFont(font);
        timerText.setString("80");
        timerText.setCharacterSize(28);
        timerText.setFillColor(sf::Color(255, 215, 0));
        timerText.setOutlineThickness(1.f);
        timerText.setOutlineColor(sf::Color::Black);

        setupAbilities();
        setupObstacles();
        resetPositions();
    }

    void open()
    {
        isActive = true;
        gameOver = false;
        playerWon = false;
        playerHealth = 100;
        enemyHealth = 100;
        gameClock.restart();
        obstacleSpawnClock.restart();
        resetPositions();
    }

    void handleInput(const sf::Event &event, const sf::Vector2f &mousePos)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
            case sf::Keyboard::W:
                keys[0] = true;
                break;
            case sf::Keyboard::A:
                keys[1] = true;
                break;
            case sf::Keyboard::S:
                keys[2] = true;
                break;
            case sf::Keyboard::D:
                keys[3] = true;
                break;
            case sf::Keyboard::Space:
                if (playerAbilityCount > 0 && abilityClock.getElapsedTime().asSeconds() > playerAbilities[0].cooldown)
                {
                    if (playerAbilityCount < MAX_ABILITIES)
                    {
                        playerAbilities[playerAbilityCount] = playerAbilities[0];
                        playerAbilities[playerAbilityCount].projectile.setPosition(
                            playerSprite.getPosition().x + playerSprite.getGlobalBounds().width,
                            playerSprite.getPosition().y + playerSprite.getGlobalBounds().height / 2);
                        playerAbilities[playerAbilityCount].sound.play();
                        playerAbilityCount++;
                        abilityClock.restart();
                    }
                }
                break;
            default:
                break;
            }
        }
        else if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
            case sf::Keyboard::W:
                keys[0] = false;
                break;
            case sf::Keyboard::A:
                keys[1] = false;
                break;
            case sf::Keyboard::S:
                keys[2] = false;
                break;
            case sf::Keyboard::D:
                keys[3] = false;
                break;
            default:
                break;
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed &&
                 event.mouseButton.button == sf::Mouse::Left)
        {
            if (closeButton.contains(mousePos))
            {
                close();
            }
        }
    }

    void update()
    {
        if (gameOver)
            return;

        int remainingTime = gameDuration - gameClock.getElapsedTime().asSeconds();
        if (remainingTime < 0)
            remainingTime = 0;
        timerText.setString(std::to_string(remainingTime));

        if (remainingTime <= 0 || playerHealth <= 0 || enemyHealth <= 0)
        {
            gameOver = true;
            playerWon = (playerHealth > enemyHealth);
            if (playerWon)
                winSound.play();
            return;
        }

        updateObstacles();

        playerVelocity.x = keys[3] ? playerSpeed : (keys[1] ? -playerSpeed : 0);
        playerVelocity.y = keys[2] ? playerSpeed : (keys[0] ? -playerSpeed : 0);

        if (playerVelocity.x != 0 && playerVelocity.y != 0)
        {
            playerVelocity *= 0.7071f;
        }

        playerSprite.move(playerVelocity);

        sf::FloatRect bounds = window.getGlobalBounds();
        sf::FloatRect playerBounds = playerSprite.getGlobalBounds();

        if (playerBounds.left < 0)
            playerSprite.setPosition(0, playerBounds.top);
        if (playerBounds.top < 50)
            playerSprite.setPosition(playerBounds.left, 50);
        if (playerBounds.left + playerBounds.width > bounds.width)
            playerSprite.setPosition(bounds.width - playerBounds.width, playerBounds.top);
        if (playerBounds.top + playerBounds.height > bounds.height - 50)
            playerSprite.setPosition(playerBounds.left, bounds.height - playerBounds.height - 50);

        // Enemy AI movement
        sf::Vector2f direction = playerSprite.getPosition() - enemySprite.getPosition();
        float distance = sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance > 200)
        {
            if (distance > 0)
            {
                direction /= distance;
                enemyVelocity = direction * enemySpeed;
            }
        }
        else if (distance < 150)
        {
            if (distance > 0)
            {
                direction /= distance;
                enemyVelocity = -direction * enemySpeed * 0.7f;
            }
        }
        else
        {
            enemyVelocity = sf::Vector2f(0, 0);
        }

        enemyVelocity.x += (rand() % 100 - 50) / 100.0f;
        enemyVelocity.y += (rand() % 100 - 50) / 100.0f;

        enemySprite.move(enemyVelocity);

        // Boundary checks
        sf::FloatRect enemyBounds = enemySprite.getGlobalBounds();

        if (enemyBounds.left < 0)
            enemySprite.setPosition(0, enemyBounds.top);
        if (enemyBounds.top < 50)
            enemySprite.setPosition(enemyBounds.left, 50);
        if (enemyBounds.left + enemyBounds.width > bounds.width)
            enemySprite.setPosition(bounds.width - enemyBounds.width, enemyBounds.top);
        if (enemyBounds.top + enemyBounds.height > bounds.height - 50)
            enemySprite.setPosition(enemyBounds.left, bounds.height - enemyBounds.height - 50);

        handleCollisions();

        for (int i = 0; i < playerAbilityCount; i++)
        {
            playerAbilities[i].projectile.move(playerAbilities[i].velocity);

            if (checkCollision(playerAbilities[i].projectile, enemySprite))
            {
                enemyHealth -= 8;
                enemyHealthText.setString(std::to_string(enemyHealth));
                hitSound.play();

                playerAbilities[i] = playerAbilities[playerAbilityCount - 1];
                playerAbilityCount--;
                i--;

                enemyHealthBar.setSize(sf::Vector2f(200 * (enemyHealth / 100.f), 20));

                if (enemyHealth <= 0)
                {
                    enemyHealth = 0;
                    enemyHealthText.setString("0");
                    gameOver = true;
                    playerWon = true;
                    winSound.play();
                    return;
                }
            }
        }

        // Enemy AI shooting
        static sf::Clock enemyShootClock;
        if (enemyShootClock.getElapsedTime().asSeconds() > 1.5f &&
            enemyAbilityCount > 0 && enemyAbilityCount < MAX_ABILITIES)
        {
            enemyAbilities[enemyAbilityCount] = enemyAbilities[0];
            enemyAbilities[enemyAbilityCount].projectile.setPosition(
                enemySprite.getPosition().x,
                enemySprite.getPosition().y + enemySprite.getGlobalBounds().height / 2);
            enemyAbilities[enemyAbilityCount].sound.play();
            enemyAbilityCount++;
            enemyShootClock.restart();
        }

        for (int i = 0; i < enemyAbilityCount; i++)
        {
            enemyAbilities[i].projectile.move(enemyAbilities[i].velocity);

            if (checkCollision(enemyAbilities[i].projectile, playerSprite))
            {
                playerHealth -= 8;
                playerHealthText.setString(std::to_string(playerHealth));
                hitSound.play();

                enemyAbilities[i] = enemyAbilities[enemyAbilityCount - 1];
                enemyAbilityCount--;
                i--;

                playerHealthBar.setSize(sf::Vector2f(200 * (playerHealth / 100.f), 20));

                if (playerHealth <= 0)
                {
                    gameOver = true;
                    playerWon = false;
                    return;
                }
            }
        }
    }

    void draw(sf::RenderWindow &targetWindow)
    {
        if (!isActive)
            return;

        sf::Vector2u winSize = targetWindow.getSize();
        backgroundDim.setSize(sf::Vector2f(winSize.x, winSize.y));
        targetWindow.draw(backgroundDim);

        sf::FloatRect windowBounds = window.getLocalBounds();
        window.setPosition((winSize.x - windowBounds.width) / 2,
                           (winSize.y - windowBounds.height) / 2);

        title.setPosition(
            window.getPosition().x + (windowBounds.width - title.getLocalBounds().width) / 2,
            window.getPosition().y + 15);

        closeButton.setPosition(
            window.getPosition().x + windowBounds.width - closeButton.getBounds().width - 20,
            window.getPosition().y + 15);

        timerText.setPosition(
            window.getPosition().x + (windowBounds.width - timerText.getLocalBounds().width) / 2,
            window.getPosition().y + 70);

        playerHealthBarBack.setPosition(
            window.getPosition().x + 20,
            window.getPosition().y + 70);

        playerHealthBar.setPosition(
            window.getPosition().x + 20,
            window.getPosition().y + 70);

        playerHealthText.setPosition(
            window.getPosition().x + 20 + (200 - playerHealthText.getLocalBounds().width) / 2,
            window.getPosition().y + 70);

        enemyHealthBarBack.setPosition(
            window.getPosition().x + windowBounds.width - 220,
            window.getPosition().y + 70);

        enemyHealthBar.setPosition(
            window.getPosition().x + windowBounds.width - 220,
            window.getPosition().y + 70);

        enemyHealthText.setPosition(
            window.getPosition().x + windowBounds.width - 220 + (200 - enemyHealthText.getLocalBounds().width) / 2,
            window.getPosition().y + 70);

        targetWindow.draw(window);
        targetWindow.draw(title);

        if (!gameOver)
        {
            for (int i = 0; i < obstacleCount; i++)
            {
                sf::Vector2f pos = obstacles[i].getPosition();
                obstacles[i].setPosition(window.getPosition().x + pos.x, window.getPosition().y + pos.y);
                targetWindow.draw(obstacles[i]);
                obstacles[i].setPosition(pos);
            }

            sf::Vector2f playerPos = playerSprite.getPosition();
            playerSprite.setPosition(window.getPosition().x + playerPos.x, window.getPosition().y + playerPos.y);
            targetWindow.draw(playerSprite);
            playerSprite.setPosition(playerPos);

            sf::Vector2f enemyPos = enemySprite.getPosition();
            enemySprite.setPosition(window.getPosition().x + enemyPos.x, window.getPosition().y + enemyPos.y);
            targetWindow.draw(enemySprite);
            enemySprite.setPosition(enemyPos);

            for (int i = 0; i < playerAbilityCount; i++)
            {
                sf::Vector2f pos = playerAbilities[i].projectile.getPosition();
                playerAbilities[i].projectile.setPosition(window.getPosition().x + pos.x, window.getPosition().y + pos.y);
                targetWindow.draw(playerAbilities[i].projectile);
                playerAbilities[i].projectile.setPosition(pos);
            }

            for (int i = 0; i < enemyAbilityCount; i++)
            {
                sf::Vector2f pos = enemyAbilities[i].projectile.getPosition();
                enemyAbilities[i].projectile.setPosition(window.getPosition().x + pos.x, window.getPosition().y + pos.y);
                targetWindow.draw(enemyAbilities[i].projectile);
                enemyAbilities[i].projectile.setPosition(pos);
            }

            targetWindow.draw(playerHealthBarBack);
            targetWindow.draw(enemyHealthBarBack);
            targetWindow.draw(playerHealthBar);
            targetWindow.draw(enemyHealthBar);
            targetWindow.draw(playerHealthText);
            targetWindow.draw(enemyHealthText);
            targetWindow.draw(timerText);
        }
        else
        {
            sf::Text gameOverText;
            gameOverText.setFont(font);
            gameOverText.setString(playerWon ? "VICTORY!" : "DEFEAT!");
            gameOverText.setCharacterSize(72);
            gameOverText.setFillColor(playerWon ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
            gameOverText.setStyle(sf::Text::Bold);
            gameOverText.setOutlineThickness(2.f);
            gameOverText.setOutlineColor(sf::Color::Black);
            gameOverText.setPosition(
                window.getPosition().x + (windowBounds.width - gameOverText.getLocalBounds().width) / 2,
                window.getPosition().y + windowBounds.height / 2 - 50);

            sf::Text scoreText;
            scoreText.setFont(font);
            scoreText.setString(playerPet->getName() + ": " + std::to_string(playerHealth) +
                                "  |  " + enemyPet->getName() + ": " + std::to_string(enemyHealth));
            scoreText.setCharacterSize(36);
            scoreText.setFillColor(sf::Color::White);
            scoreText.setOutlineThickness(1.f);
            scoreText.setOutlineColor(sf::Color::Black);
            scoreText.setPosition(
                window.getPosition().x + (windowBounds.width - scoreText.getLocalBounds().width) / 2,
                window.getPosition().y + windowBounds.height / 2 + 20);

            targetWindow.draw(gameOverText);
            targetWindow.draw(scoreText);
        }

        closeButton.draw(targetWindow);
    }
                                                // getter
    bool isOpen() const { return isActive; }
    void close() { isActive = false; }
};

// ==================== BATTLE SELECTION WINDOW CLASS==================== //

// Battle type selection menu (1v1/2v2/Guild) with interactive buttons.
// Uses encapsulation to hide UI details and abstraction through simple open and close methods keeping battle logic separate from menu handling.
class BattleSelectionWindow
{
private:
    sf::RectangleShape window;
    sf::RectangleShape backgroundDim;
    sf::Text title;
    Button backButton;
    Button battleButtons[3];
    bool isActive;
    sf::Font font;
    int selectedBattle;
    bool backClickedFlag;

public:
    BattleSelectionWindow() : isActive(false), selectedBattle(-1), backClickedFlag(false) {} // constructor

    void setup(const sf::Font &gameFont)
    {
        font = gameFont;

        backgroundDim.setFillColor(sf::Color(0, 0, 0, 180));

        window.setSize(sf::Vector2f(600, 400));
        window.setFillColor(sf::Color(40, 40, 50, 240));
        window.setOutlineThickness(4.f);
        window.setOutlineColor(sf::Color(255, 215, 0));

        title.setFont(font);
        title.setString("SELECT BATTLE TYPE");
        title.setCharacterSize(42);
        title.setFillColor(sf::Color(255, 215, 0));
        title.setStyle(sf::Text::Bold);
        title.setOutlineThickness(2.f);
        title.setOutlineColor(sf::Color::Black);

        backButton = Button(font, "BACK", 28,
                            sf::Color(255, 215, 0),
                            sf::Color(255, 255, 150),
                            sf::Color(200, 170, 0));

        battleButtons[0] = Button(font, "1 vs 1 BATTLE", 32,
                                  sf::Color(255, 100, 100),
                                  sf::Color(255, 150, 150),
                                  sf::Color(200, 50, 50));

        battleButtons[1] = Button(font, "2 vs 2 BATTLE", 32,
                                  sf::Color(100, 100, 255),
                                  sf::Color(150, 150, 255),
                                  sf::Color(50, 50, 200));
    }

    void open()
    {
        isActive = true;
        selectedBattle = -1;
    }

    void close() { isActive = false; }
    bool isOpen() const { return isActive; }

    void update(const sf::Vector2f &mousePos)
    {
        backButton.update(mousePos);

        for (int i = 0; i < 3; i++)
        {
            battleButtons[i].update(mousePos);
            if (battleButtons[i].contains(mousePos))
            {
                selectedBattle = i;
            }
        }
    }

    void handleInput(const sf::Event &event, const sf::Vector2f &mousePos)
    {
        if (!isActive)
            return;

        if (!contains(mousePos))
            return;

        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left)
        {
            if (backButton.contains(mousePos))
            {
                backClickedFlag = true;
                close();
                return;
            }
            else if (selectedBattle >= 0 && selectedBattle < 3)
            {
                std::cout << "Selected battle type: " << selectedBattle << std::endl;
                close();
                return;
            }
        }
    }
    void draw(sf::RenderWindow &targetWindow)
    {
        if (!isActive)
            return;

        sf::Vector2u winSize = targetWindow.getSize();
        backgroundDim.setSize(sf::Vector2f(winSize.x, winSize.y));
        targetWindow.draw(backgroundDim);

        sf::FloatRect windowBounds = window.getLocalBounds();
        window.setPosition((winSize.x - windowBounds.width) / 2,
                           (winSize.y - windowBounds.height) / 2);

        title.setPosition(window.getPosition().x + (windowBounds.width - title.getLocalBounds().width) / 2,
                          window.getPosition().y + 30);
        targetWindow.draw(window);
        targetWindow.draw(title);

        float startY = window.getPosition().y + 120;
        float spacing = 80;

        for (int i = 0; i < 3; i++)
        {
            battleButtons[i].setPosition(
                window.getPosition().x + (windowBounds.width - battleButtons[i].getBounds().width) / 2,
                startY + i * spacing);
            battleButtons[i].draw(targetWindow);
        }

        backButton.setPosition(
            window.getPosition().x + (windowBounds.width - backButton.getBounds().width) / 2,
            window.getPosition().y + windowBounds.height - 80);
        backButton.draw(targetWindow);
    }
                                                // getter
    int getSelectedBattle() const { return selectedBattle; }

    bool contains(const sf::Vector2f &point) const
    {
        return window.getGlobalBounds().contains(point);
    }

    bool backClicked() const { return backClickedFlag; }
    void resetBackFlag() { backClickedFlag = false; }
};

// ==================== TRAINING GAME CLASS ==================== //

// Interactive pet training minigame where players dodge projectiles and shoot targets to earn XP.
// Uses encapsulation  and composition contains Pet* trainedPet as a member has a relationship)

class TrainingGame
{
private:
    static const int MAX_PROJECTILES = 50;

    sf::RectangleShape window;
    sf::RectangleShape backgroundDim;
    sf::Text title;
    Button closeButton;
    Button backButton;
    sf::Font font;

    sf::Texture playerTexture;
    sf::Sprite playerSprite;
    sf::Texture enemyTexture;
    sf::Sprite enemySprite;

    sf::Texture playerProjectileTexture;
    sf::Sprite playerProjectiles[MAX_PROJECTILES];
    int playerProjectileCount;

    sf::Texture enemyProjectileTexture;
    sf::Sprite enemyProjectiles[MAX_PROJECTILES];
    int enemyProjectileCount;

    int playerScore;
    int enemyScore;
    sf::Text playerScoreText;
    sf::Text enemyScoreText;
    sf::Text timerText;
    sf::Text gameOverText;
    sf::Text resultText;
    Button continueButton;

    sf::Clock projectileClock;
    sf::Clock enemyMoveClock;
    sf::Clock gameTimer;
    bool isActive;
    float playerSpeed;
    float enemySpeed;
    int gameDuration;
    bool gameOver;
    Pet *trainedPet;

    int oldLevel;

    sf::SoundBuffer fireSoundBuffer;
    sf::Sound fireSound;
    sf::SoundBuffer hitSoundBuffer;
    sf::Sound hitSound;

    void removeBackground(sf::Image &image, const std::string &filename)
    {
        if (filename.find("unicorn") != std::string::npos)
        {
            return;
        }

        sf::Color backgroundColor = image.getPixel(0, 0);

        for (unsigned int y = 0; y < image.getSize().y; ++y)
        {
            for (unsigned int x = 0; x < image.getSize().x; ++x)
            {
                sf::Color pixel = image.getPixel(x, y);
                if (abs(pixel.r - backgroundColor.r) < 30 &&
                    abs(pixel.g - backgroundColor.g) < 30 &&
                    abs(pixel.b - backgroundColor.b) < 30)
                {
                    pixel.a = 0;
                    image.setPixel(x, y, pixel);
                }
            }
        }
    }

    void resetPositions()
    {
        float playerX = window.getPosition().x + window.getSize().x * 0.21f;
        float playerY = window.getPosition().y + (window.getSize().y - playerSprite.getGlobalBounds().height) / 2;
        playerSprite.setPosition(playerX, playerY);

        float enemyX = window.getPosition().x + window.getSize().x * 1.2f;
        float enemyY = window.getPosition().y + (window.getSize().y - enemySprite.getGlobalBounds().height) / 2;
        enemySprite.setPosition(enemyX, enemyY);
    }

    void addPlayerProjectile()
    {
        if (playerProjectileCount < MAX_PROJECTILES)
        {
            playerProjectiles[playerProjectileCount] = sf::Sprite(playerProjectileTexture);
            playerProjectiles[playerProjectileCount].setPosition(
                playerSprite.getPosition().x + playerSprite.getGlobalBounds().width,
                playerSprite.getPosition().y + playerSprite.getGlobalBounds().height / 2 - 15);
            playerProjectiles[playerProjectileCount].setScale(0.4f, 0.4f);
            playerProjectileCount++;
        }
    }

    void addEnemyProjectile()
    {
        if (enemyProjectileCount < MAX_PROJECTILES)
        {
            enemyProjectiles[enemyProjectileCount] = sf::Sprite(enemyProjectileTexture);
            enemyProjectiles[enemyProjectileCount].setPosition(
                enemySprite.getPosition().x,
                enemySprite.getPosition().y + enemySprite.getGlobalBounds().height / 2 - 15);
            enemyProjectiles[enemyProjectileCount].setScale(0.4f, 0.4f);
            enemyProjectileCount++;
        }
    }

    void setupTextElements()
    {
        playerScoreText.setFont(font);
        playerScoreText.setString(trainedPet->getName() + ": 0");
        playerScoreText.setCharacterSize(24);
        playerScoreText.setFillColor(sf::Color(255, 150, 100));
        playerScoreText.setOutlineThickness(1.f);
        playerScoreText.setOutlineColor(sf::Color::Black);

        enemyScoreText.setFont(font);
        enemyScoreText.setString("ENEMY: 0");
        enemyScoreText.setCharacterSize(24);
        enemyScoreText.setFillColor(sf::Color(100, 150, 255));
        enemyScoreText.setOutlineThickness(1.f);
        enemyScoreText.setOutlineColor(sf::Color::Black);

        timerText.setFont(font);
        timerText.setString("TIME: " + std::to_string(gameDuration));
        timerText.setCharacterSize(28);
        timerText.setFillColor(sf::Color(255, 215, 0));
        timerText.setOutlineThickness(1.f);
        timerText.setOutlineColor(sf::Color::Black);

        gameOverText.setFont(font);
        gameOverText.setString("TRAINING COMPLETE!");
        gameOverText.setCharacterSize(36);
        gameOverText.setFillColor(sf::Color(255, 215, 0));
        gameOverText.setOutlineThickness(2.f);
        gameOverText.setOutlineColor(sf::Color::Black);

        resultText.setFont(font);
        resultText.setCharacterSize(28);
        resultText.setFillColor(sf::Color::White);
        resultText.setOutlineThickness(1.f);
        resultText.setOutlineColor(sf::Color::Black);

        continueButton = Button(font, "CONTINUE", 32,
                                sf::Color(100, 200, 100),
                                sf::Color(150, 250, 150),
                                sf::Color(50, 150, 50));

        backButton = Button(font, "BACK", 28,
                            sf::Color(255, 100, 100),
                            sf::Color(255, 150, 150),
                            sf::Color(200, 50, 50));
    }

public:
    TrainingGame() : isActive(false), playerScore(0), enemyScore(0),                // constructor
                     playerSpeed(0), enemySpeed(1.5f), gameDuration(60),
                     gameOver(false), trainedPet(nullptr),
                     playerProjectileCount(0), enemyProjectileCount(0),
                     oldLevel(1)
    {
    }

    std::string getRandomEnemyTexture(Pet *playerPet)
    {
        std::string playerType = playerPet->getType();
        int count = 0;
        std::string enemies[4];

        if (playerType != "Fire")
            enemies[count++] = "dragon1.png";
        if (playerType != "Ice")
            enemies[count++] = "phoneix1.png";

        return (count == 0) ? "dragon1.png" : enemies[rand() % count];
    }

    void setup(const sf::Font &gameFont, Pet *petToTrain)
    {
        font = gameFont;
        trainedPet = petToTrain;

        backgroundDim.setFillColor(sf::Color(0, 0, 0, 180));
        window.setSize(sf::Vector2f(900, 600));
        window.setFillColor(sf::Color(40, 40, 50, 240));
        window.setOutlineThickness(4.f);
        window.setOutlineColor(sf::Color(255, 215, 0));

        title.setFont(font);
        title.setString("PET TRAINING ARENA");
        title.setCharacterSize(42);
        title.setFillColor(sf::Color(255, 215, 0));
        title.setStyle(sf::Text::Bold);
        title.setOutlineThickness(2.f);
        title.setOutlineColor(sf::Color::Black);

        closeButton = Button(font, "CLOSE", 28,
                             sf::Color(255, 100, 100),
                             sf::Color(255, 150, 150),
                             sf::Color(200, 50, 50));

        std::string playerTexFile, projectileTexFile;

        if (trainedPet->getType() == "Fire")
        {
            playerTexFile = "dragon1.png";
            projectileTexFile = "fire1.png";
        }
        else if (trainedPet->getType() == "Ice")
        {
            playerTexFile = "phoneix1.png";
            projectileTexFile = "ice1.png";
        }
        else if (trainedPet->getType() == "Electric")
        {
            playerTexFile = "griffin1.png";
            projectileTexFile = "lightning.png";
        }
        else
        {
            playerTexFile = "unicorn1.png";
            projectileTexFile = "magic.png";
        }

        if (!playerTexture.loadFromFile(playerTexFile))
        {
            sf::Image placeholder;
            placeholder.create(150, 150, sf::Color::Magenta);
            playerTexture.loadFromImage(placeholder);
        }
        else
        {
            sf::Image playerImg = playerTexture.copyToImage();
            removeBackground(playerImg, playerTexFile); 
            playerTexture.loadFromImage(playerImg);
        }

        playerSprite.setTexture(playerTexture);
        float scale = (window.getSize().y * 0.12f) / playerSprite.getLocalBounds().height;
        playerSprite.setScale(scale, scale);

        std::string enemyTexFile = getRandomEnemyTexture(trainedPet);
        if (!enemyTexture.loadFromFile(enemyTexFile))
        {
            sf::Image placeholder;
            placeholder.create(150, 150, sf::Color::Cyan);
            enemyTexture.loadFromImage(placeholder);
        }
        else
        {
            sf::Image enemyImg = enemyTexture.copyToImage();
            removeBackground(enemyImg, enemyTexFile); 
            enemyTexture.loadFromImage(enemyImg);
        }
        enemySprite.setTexture(enemyTexture);
        enemySprite.setScale(scale, scale);

        if (!playerProjectileTexture.loadFromFile(projectileTexFile))
        {
            sf::Image placeholder;
            placeholder.create(50, 20, sf::Color::Yellow);
            playerProjectileTexture.loadFromImage(placeholder);
        }

        if (!enemyProjectileTexture.loadFromFile("lightning.png"))
        {
            sf::Image placeholder;
            placeholder.create(50, 20, sf::Color::Blue);
            enemyProjectileTexture.loadFromImage(placeholder);
        }

        if (!fireSoundBuffer.loadFromFile("fire.wav"))
        {
            std::cerr << "Error loading fire sound!" << std::endl;
        }
        fireSound.setBuffer(fireSoundBuffer);

        if (!hitSoundBuffer.loadFromFile("hit.wav"))
        {
            std::cerr << "Error loading hit sound!" << std::endl;
        }
        hitSound.setBuffer(hitSoundBuffer);

        setupTextElements();
        resetPositions();
    }

    void update(const sf::Vector2f &mousePos)
    {
        if (!isActive)
            return;

        bool mouseInTitleArea = mousePos.y < window.getPosition().y + 60;
        if (mouseInTitleArea)
        {
            closeButton.update(mousePos);
            backButton.update(mousePos);
        }
        else
        {
            closeButton.setActive(false);
            backButton.setActive(false);
        }

        if (gameOver)
        {
            continueButton.update(mousePos);
            return;
        }

        int remainingTime = gameDuration - gameTimer.getElapsedTime().asSeconds();
        remainingTime = std::max(0, remainingTime);
        timerText.setString("TIME: " + std::to_string(remainingTime));

        if (remainingTime <= 0 && !gameOver)
        {
            endGame();
            return;
        }

        float newY = mousePos.y - playerSprite.getGlobalBounds().height / 2;
        float minY = window.getPosition().y + 80;
        float maxY = window.getPosition().y + window.getSize().y - playerSprite.getGlobalBounds().height - 80;
        playerSprite.setPosition(playerSprite.getPosition().x, std::clamp(newY, minY, maxY));

        // Enemy AI movement
        if (enemyMoveClock.getElapsedTime().asSeconds() > 0.5f)
        {
            int aiChoice = rand() % 100;
            if (aiChoice < 40)
            {
                enemySpeed = (playerSprite.getPosition().y - enemySprite.getPosition().y) * 0.05f;
            }
            else if (aiChoice < 70)
            {
                enemySpeed = ((rand() % 100) / 50.0f) - 1.0f;
            }
            else
            {
                enemySpeed = 0;
            }
            enemyMoveClock.restart();
        }

        enemySprite.move(0, enemySpeed * 3.0f);
        float enemyMinY = window.getPosition().y + 80;
        float enemyMaxY = window.getPosition().y + window.getSize().y - enemySprite.getGlobalBounds().height - 80;
        enemySprite.setPosition(enemySprite.getPosition().x,
                                std::clamp(enemySprite.getPosition().y, enemyMinY, enemyMaxY));

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && projectileClock.getElapsedTime().asSeconds() > 0.2f)
        {
            addPlayerProjectile();
            projectileClock.restart();
        }

        // Enemy shooting
        if (rand() % 100 < 2 + trainedPet->getLevel() / 5)
        {
            addEnemyProjectile();
        }

        updateProjectiles();
    }

    void updateProjectiles()
    {
        float playerProjectileSpeed = 15.0f + trainedPet->getSpeed() * 0.5f;
        for (int i = 0; i < playerProjectileCount; i++)
        {
            playerProjectiles[i].move(playerProjectileSpeed, 0);
        }

        float enemyProjectileSpeed = 12.0f;
        for (int i = 0; i < enemyProjectileCount; i++)
        {
            enemyProjectiles[i].move(-enemyProjectileSpeed, 0);
        }

        auto removeIf = [](auto &array, int &count, auto predicate)
        {
            for (int i = 0; i < count;)
            {
                if (predicate(array[i]))
                {
                    for (int j = i; j < count - 1; j++)
                        array[j] = array[j + 1];
                    count--;
                }
                else
                {
                    i++;
                }
            }
        };

        removeIf(playerProjectiles, playerProjectileCount, [this](const auto &proj)
                 { return proj.getPosition().x > window.getPosition().x + window.getSize().x; });

        removeIf(enemyProjectiles, enemyProjectileCount, [this](const auto &proj)
                 { return proj.getPosition().x + proj.getGlobalBounds().width < window.getPosition().x; });

        checkCollisions(); // check for collisions
    }

    void checkCollisions()
    {
        sf::FloatRect enemyRect = enemySprite.getGlobalBounds();
        for (int i = 0; i < playerProjectileCount;)
        {
            if (enemyRect.intersects(playerProjectiles[i].getGlobalBounds()))
            {
                playerScore += 10 + trainedPet->getAttack() / 2;
                playerScoreText.setString(trainedPet->getName() + ": " + std::to_string(playerScore));

                for (int j = i; j < playerProjectileCount - 1; j++)
                {
                    playerProjectiles[j] = playerProjectiles[j + 1];
                }
                playerProjectileCount--;
            }
            else
            {
                i++;
            }
        }

        sf::FloatRect playerRect = playerSprite.getGlobalBounds();
        for (int i = 0; i < enemyProjectileCount;)
        {
            if (playerRect.intersects(enemyProjectiles[i].getGlobalBounds()))
            {
                enemyScore += 10;
                enemyScoreText.setString("ENEMY: " + std::to_string(enemyScore));

                for (int j = i; j < enemyProjectileCount - 1; j++)
                {
                    enemyProjectiles[j] = enemyProjectiles[j + 1];
                }
                enemyProjectileCount--;
            }
            else
            {
                i++;
            }
        }
    }

    void endGame()
    {
        gameOver = true;

        int baseExp = std::min(playerScore, 50);
        int timeBonus = (gameDuration - gameTimer.getElapsedTime().asSeconds()) / 2;
        int totalExp = baseExp + timeBonus;

        totalExp = std::min(totalExp, 80);
        totalExp = std::max(totalExp, 10);

        trainedPet->gainExperience(totalExp);
        bool leveledUp = (trainedPet->getLevel() > oldLevel);

        resultText.setString("Training Points: " + std::to_string(totalExp));

        continueButton.setPosition(
            window.getPosition().x + window.getSize().x / 2 - continueButton.getBounds().width / 2,
            window.getPosition().y + window.getSize().y - 100);

        trainedPet->updateStats();
    }

    void draw(sf::RenderWindow &targetWindow)
    {
        if (!isActive)
            return;

        sf::Vector2u winSize = targetWindow.getSize();
        backgroundDim.setSize(sf::Vector2f(winSize.x, winSize.y));
        targetWindow.draw(backgroundDim);

        window.setPosition((winSize.x - window.getSize().x) / 2,
                           (winSize.y - window.getSize().y) / 2);

        targetWindow.draw(window);
        title.setPosition(window.getPosition().x + (window.getSize().x - title.getLocalBounds().width) / 2,
                          window.getPosition().y + 15);
        targetWindow.draw(title);

        if (!gameOver)
        {
            playerScoreText.setPosition(
                window.getPosition().x + 50,
                window.getPosition().y + window.getSize().y - 50);

            enemyScoreText.setPosition(
                window.getPosition().x + window.getSize().x - enemyScoreText.getLocalBounds().width - 50,
                window.getPosition().y + window.getSize().y - 50);

            timerText.setPosition(
                window.getPosition().x + (window.getSize().x - timerText.getLocalBounds().width) / 2,
                window.getPosition().y + 70);

            targetWindow.draw(playerScoreText);
            targetWindow.draw(enemyScoreText);
            targetWindow.draw(timerText);

            targetWindow.draw(playerSprite);
            targetWindow.draw(enemySprite);

            for (int i = 0; i < playerProjectileCount; i++)
                targetWindow.draw(playerProjectiles[i]);
            for (int i = 0; i < enemyProjectileCount; i++)
                targetWindow.draw(enemyProjectiles[i]);
        }
        else
        {
            gameOverText.setPosition(
                window.getPosition().x + (window.getSize().x - gameOverText.getLocalBounds().width) / 2,
                window.getPosition().y + window.getSize().y / 2 - 60);

            resultText.setPosition(
                window.getPosition().x + (window.getSize().x - resultText.getLocalBounds().width) / 2,
                window.getPosition().y + window.getSize().y / 2);

            targetWindow.draw(gameOverText);
            targetWindow.draw(resultText);
            continueButton.draw(targetWindow);
        }

        if (sf::Mouse::getPosition(targetWindow).y < window.getPosition().y + 60)
        {
            closeButton.setPosition(window.getPosition().x + 20, window.getPosition().y + 15);
            backButton.setPosition(window.getPosition().x + window.getSize().x - backButton.getBounds().width - 20,
                                   window.getPosition().y + 15);
            closeButton.draw(targetWindow);
            backButton.draw(targetWindow);
        }
    }

    void open(Pet *petToTrain)
    {
        isActive = true;
        playerScore = 0;
        enemyScore = 0;
        playerProjectileCount = 0;
        enemyProjectileCount = 0;
        gameOver = false;
        trainedPet = petToTrain;
        oldLevel = trainedPet->getLevel();

        projectileClock.restart();
        enemyMoveClock.restart();
        gameTimer.restart();

        setup(font, petToTrain);
        resetPositions();

        window.setPosition((800 - window.getSize().x) / 2, (600 - window.getSize().y) / 2);
    }

    void handleInput(const sf::Event &event, const sf::Vector2f &mousePos)
    {
        if (!isActive)
            return;

        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left)
        {
            if (mousePos.y < window.getPosition().y + 60)
            {
                if (closeButton.contains(mousePos) || backButton.contains(mousePos))
                {
                    close();
                }
            }
            else if (gameOver && continueButton.contains(mousePos))
            {
                close();
            }
        }
    }

    bool isOpen() const { return isActive; }
    void close()
    {
        isActive = false;
        playerProjectileCount = 0;
        enemyProjectileCount = 0;
    }
};

// ==================== ITEM CLASS ==================== //

// Base class for all game items potions etc.
//  Uses inheritance for future servided classes can inherit form it  and encapsulation hiding details
class Item
{
protected:
    std::string name;
    std::string description;
    int price;
    int quantity;
    sf::Texture texture;
    sf::Sprite sprite;

public:
    Item() : name(""), description(""), price(0), quantity(0) {}            // constructor

    Item(const std::string &n, const std::string &desc, int p, const std::string &textureFile)
        : name(n), description(desc), price(p), quantity(0)
    {
        if (!texture.loadFromFile(textureFile))
        {
            sf::Image placeholder;
            placeholder.create(64, 64, sf::Color::Magenta);
            texture.loadFromImage(placeholder);
        }
        sprite.setTexture(texture);
        sprite.setScale(0.8f, 0.8f);
    }

    virtual ~Item() {}                      // polymorphism
                                        // getter
    std::string getName() const { return name; }
    std::string getDescription() const { return description; }
    int getPrice() const { return price; }
    int getQuantity() const { return quantity; }

    void setQuantity(int q) { quantity = q; }
    void addQuantity(int amount) { quantity += amount; }

    virtual bool use()
    {
        if (quantity > 0)
        {
            quantity--;
            return true;
        }
        return false;
    }

    void draw(sf::RenderWindow &window, float x, float y)
    {
        sprite.setPosition(x, y);
        window.draw(sprite);
    }
};

//   -----------  Health Potion Class --------------//

// Health potion item that restores 50 HP when used.
// Uses inheritance to extend Item class and override the use() method with healing logic
class HealthPotion : public Item
{
public:
    HealthPotion() : Item("Health Potion", "Restores 50 HP", 100, "healing1.png") {} // constructor

    bool use() override
    {
        if (Item::use())
        {
            std::cout << "Used Health Potion - Restored 50 HP" << std::endl;
            return true;
        }
        return false;
    }
};

// -------- Energy Potion Class ----------//

// Energy potion that restores ability energy when consumed.
// Inherits from Item and overrides use() to implement energy restoration

class EnergyPotion : public Item
{
public:
    EnergyPotion() : Item("Energy Potion", "Restores ability energy", 150, "potion1.png") {}                    // constructor

    bool use() override
    {
        if (Item::use())
        {
            std::cout << "Used Energy Potion - Restored energy" << std::endl;
            return true;
        }
        return false;
    }
};

//----------- Attack Buff Class ------------------//

// Temporary attack booster item .
// Inherits from Item and overrides use() to apply the buff effect
class AttackBuff : public Item
{
public:
    AttackBuff() : Item("Attack Buff", "+20% attack for 3 turns", 200, "buff1.png") {}                  // constructor

    bool use() override
    {
        if (Item::use())
        {
            std::cout << "Used Attack Buff - Increased attack by 20%" << std::endl;
            return true;
        }
        return false;
    }
};

// ------------ Speed Buff Class -------------//

// Temporary speed booster item .
// Uses inheritance from Item and overrides use() to apply movement buffs.
class SpeedBuff : public Item
{
public:
    SpeedBuff() : Item("Speed Buff", "+20% speed for 3 turns", 200, "buff2.png") {}             // constructor

    bool use() override
    {
        if (Item::use())
        {
            std::cout << "Used Speed Buff - Increased speed by 20%" << std::endl;
            return true;
        }
        return false;
    }
};

// Defensive potion that reduces incoming damage by 30%.
// Inherits from Item and overrides use() to apply damage reduction
class ShieldPotion : public Item
{
public:
    ShieldPotion() : Item("Shield Potion", "Reduces damage by 30%", 250, "shield1.png") {}              // constructor

    bool use() override
    {
        if (Item::use())
        {
            std::cout << "Used Shield Potion - Damage reduced by 30%" << std::endl;
            return true;
        }
        return false;
    }
};

//     ------ INVENTORY MAIN CLASS -------------//

// It Manages player's items and shop - handles buying, storing, and displaying potions/buffs.
// It  uses encapsulation  and composition  for clean inventory logic.

class Inventory
{
private:
    static const int MAX_ITEMS = 5;
    Item *items[MAX_ITEMS];
    sf::Font font;
    sf::Text title;
    sf::Text diamondText;
    sf::Sprite diamondSprite;
    Button closeButton;
    Button buyButtons[MAX_ITEMS];
    sf::RectangleShape window;
    sf::RectangleShape backgroundDim;
    bool isActive;
    UserData *userData;
    sf::Texture diamondTexture;

public:
    Inventory() : isActive(false), userData(nullptr) // constructor
    {
        for (int i = 0; i < MAX_ITEMS; i++)
        {
            items[i] = nullptr;
        }
    }

    ~Inventory() // destructor
    {
        for (int i = 0; i < MAX_ITEMS; i++)
        {
            delete items[i];
        }
    }

    void setup(const sf::Font &gameFont, UserData *data)
    {
        font = gameFont;
        userData = data;

        items[0] = new HealthPotion();
        items[1] = new EnergyPotion();
        items[2] = new AttackBuff();
        items[3] = new SpeedBuff();
        items[4] = new ShieldPotion();

        for (int i = 0; i < MAX_ITEMS; i++)
        {
            if (items[i])
            {
                items[i]->setQuantity(userData->getItemQuantity(i));
            }
        }

        backgroundDim.setFillColor(sf::Color(0, 0, 0, 180));
        window.setSize(sf::Vector2f(900, 700));
        window.setFillColor(sf::Color(40, 40, 50, 240));
        window.setOutlineThickness(4.f);
        window.setOutlineColor(sf::Color(255, 215, 0));

        title.setFont(font);
        title.setString("INVENTORY & SHOP");
        title.setCharacterSize(42);
        title.setFillColor(sf::Color(255, 215, 0));
        title.setStyle(sf::Text::Bold);
        title.setOutlineThickness(2.f);
        title.setOutlineColor(sf::Color::Black);

        diamondText.setFont(font);
        diamondText.setCharacterSize(32);
        diamondText.setFillColor(sf::Color(100, 255, 255));
        diamondText.setOutlineThickness(1.f);
        diamondText.setOutlineColor(sf::Color::Black);

        if (!diamondTexture.loadFromFile("diamond.png"))
        {
            sf::Image img;
            img.create(32, 32, sf::Color::Transparent);
            for (int y = 0; y < 16; y++)
            {
                for (int x = 15 - y; x <= 15 + y; x++)
                {
                    img.setPixel(x, y, sf::Color(100, 255, 255));
                }
            }
            for (int y = 16; y < 32; y++)
            {
                for (int x = y - 16; x <= 47 - y; x++)
                {
                    img.setPixel(x, y, sf::Color(100, 255, 255));
                }
            }
            diamondTexture.loadFromImage(img);
        }
        diamondSprite.setTexture(diamondTexture);
        diamondSprite.setScale(0.8f, 0.8f);

        closeButton = Button(font, "CLOSE", 28,
                             sf::Color(255, 100, 100),
                             sf::Color(255, 150, 150),
                             sf::Color(200, 50, 50));

        for (int i = 0; i < MAX_ITEMS; i++)
        {
            buyButtons[i] = Button(font, "  BUY  ", 28,
                                   sf::Color(100, 200, 100),
                                   sf::Color(150, 250, 150),
                                   sf::Color(50, 150, 50));
        }
    }

    void open()
    {
        isActive = true;
        updateDiamondDisplay();
        for (int i = 0; i < MAX_ITEMS; i++)
        {
            if (items[i])
            {
                items[i]->setQuantity(userData->getItemQuantity(i));
            }
        }
    }

    void close()
    {
        isActive = false;
        for (int i = 0; i < MAX_ITEMS; i++)
        {
            if (items[i])
            {
                userData->setItemQuantity(i, items[i]->getQuantity());
            }
        }
        userData->updateUserData();
    }

    bool isOpen() const { return isActive; }

    void updateDiamondDisplay()
    {
        if (userData)
        {
            diamondText.setString(std::to_string(userData->getDiamonds()));
        }
    }

    void update(const sf::Vector2f &mousePos)
    {
        if (!isActive)
            return;

        closeButton.update(mousePos);

        for (int i = 0; i < MAX_ITEMS; i++)
        {
            buyButtons[i].update(mousePos);
        }
    }

    void handleInput(const sf::Event &event, const sf::Vector2f &mousePos)
    {
        if (!isActive)
            return;

        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left)
        {
            if (closeButton.contains(mousePos))
            {
                close();
            }

            for (int i = 0; i < MAX_ITEMS; i++)
            {
                if (!items[i])
                    continue;

                if (buyButtons[i].contains(mousePos) && userData)
                {
                    if (userData->getDiamonds() >= items[i]->getPrice())
                    {
                        userData->addDiamonds(-items[i]->getPrice());
                        items[i]->addQuantity(1);
                        userData->setItemQuantity(i, items[i]->getQuantity());
                        updateDiamondDisplay();
                    }
                }
            }
        }
    }

    void draw(sf::RenderWindow &targetWindow)
    {
        if (!isActive)
            return;

        sf::Vector2u winSize = targetWindow.getSize();
        backgroundDim.setSize(sf::Vector2f(winSize.x, winSize.y));
        targetWindow.draw(backgroundDim);

        sf::FloatRect windowBounds = window.getLocalBounds();
        window.setPosition((winSize.x - windowBounds.width) / 2,
                           (winSize.y - windowBounds.height) / 2);

        title.setPosition(window.getPosition().x + (windowBounds.width - title.getLocalBounds().width) / 2,
                          window.getPosition().y + 30);
        targetWindow.draw(window);
        targetWindow.draw(title);

        diamondText.setPosition(window.getPosition().x + windowBounds.width - 100,
                                window.getPosition().y + 30);
        diamondSprite.setPosition(diamondText.getPosition().x - 40,
                                  diamondText.getPosition().y);
        targetWindow.draw(diamondSprite);
        targetWindow.draw(diamondText);

        closeButton.setPosition(
            window.getPosition().x + 30,
            window.getPosition().y + 30);
        closeButton.draw(targetWindow);

        float startY = window.getPosition().y + 100;
        float spacing = 120;

        for (int i = 0; i < MAX_ITEMS; i++)
        {
            if (!items[i])
                continue;

            float yPos = startY + i * spacing;

            items[i]->draw(targetWindow, window.getPosition().x + 40, yPos);

            float infoX = window.getPosition().x + 150;

            sf::Text nameText(items[i]->getName(), font, 26);
            nameText.setFillColor(sf::Color(255, 255, 200));
            nameText.setPosition(infoX, yPos);
            targetWindow.draw(nameText);

            sf::Text descText(items[i]->getDescription(), font, 20);
            descText.setFillColor(sf::Color(200, 200, 255));
            descText.setPosition(infoX, yPos + 30);
            targetWindow.draw(descText);

            sf::Text priceText("Price: " + std::to_string(items[i]->getPrice()) + " diamonds", font, 20);
            priceText.setFillColor(sf::Color(255, 215, 0));
            priceText.setPosition(infoX, yPos + 60);
            targetWindow.draw(priceText);

            sf::Text qtyText("Owned: " + std::to_string(items[i]->getQuantity()), font, 20);
            qtyText.setFillColor(sf::Color(100, 255, 100));
            qtyText.setPosition(priceText.getPosition().x + 250, yPos + 60);
            targetWindow.draw(qtyText);

            buyButtons[i].setPosition(window.getPosition().x + windowBounds.width - 150, yPos + 20);
            buyButtons[i].draw(targetWindow);

            if (i < MAX_ITEMS - 1)
            {
                sf::RectangleShape separator(sf::Vector2f(windowBounds.width - 80, 2));
                separator.setFillColor(sf::Color(100, 100, 100, 100));
                separator.setPosition(window.getPosition().x + 40, yPos + spacing - 20);
                targetWindow.draw(separator);
            }
        }
    }
};

// ==================== SCOREBOARD CLASS ==================== //

// This class manages a leaderboard of top 5 players by diamonds, using file I/O to persist data
// It uses encapsulation to hide file operations and operator overloading for comparisons
class Scoreboard
{
private:
    static const int MAX_ENTRIES = 5;
    struct Entry
    {
        std::string username;
        int diamonds;

        bool operator<(const Entry &other) const
        {                                                   // opeartor overloading to compare diamonds
            return diamonds > other.diamonds;
        }
    };

    Entry entries[MAX_ENTRIES];
    sf::Font font;
    std::string currentPlayer;
    int currentDiamonds;

    void loadAndSortEntries()
    {
        for (int i = 0; i < MAX_ENTRIES; i++)
        {
            entries[i] = {"-----", 0};
        }

        std::ifstream file("user_data.txt");
        if (!file.is_open())
            return;

        std::string line;
        int entryCount = 0;
        Entry allEntries[100];
        int totalUsers = 0;

        while (std::getline(file, line))
        {
            if (line.find("Username: ") == 0)
            {
                std::string username = line.substr(10);

                                            // Get diamonds
                std::getline(file, line);
                int diamonds = 0;
                if (line.find("Diamonds: ") == 0)
                {
                    diamonds = std::stoi(line.substr(10));
                }

                for (int i = 0; i < 9 && std::getline(file, line); i++)
                    ;

                allEntries[totalUsers++] = {username, diamonds};
            }
        }
        file.close();

        bool currentPlayerExists = false;
        for (int i = 0; i < totalUsers; i++)
        {
            if (allEntries[i].username == currentPlayer)
            {
                currentPlayerExists = true;
                if (currentDiamonds > allEntries[i].diamonds)
                {
                    allEntries[i].diamonds = currentDiamonds;
                }
                break;
            }
        }
        if (!currentPlayerExists && totalUsers < 100)
        {
            allEntries[totalUsers++] = {currentPlayer, currentDiamonds};
        }

        for (int i = 0; i < totalUsers - 1; i++)
        {                                                                   // sorting
            for (int j = 0; j < totalUsers - i - 1; j++)
            {
                if (allEntries[j + 1] < allEntries[j])
                {
                    Entry temp = allEntries[j];
                    allEntries[j] = allEntries[j + 1];
                    allEntries[j + 1] = temp;
                }
            }
        }

        for (int i = 0; i < MAX_ENTRIES && i < totalUsers; i++)
        {                                                                       // only 5
            entries[i] = allEntries[i];
        }
    }

public:
    Scoreboard()
    {
        currentPlayer = "";
        currentDiamonds = 0;
        loadAndSortEntries();
    }

    void setup(const sf::Font &gameFont, const std::string &playerName, int diamonds)
    {
        font = gameFont;
        currentPlayer = playerName;
        currentDiamonds = diamonds;
        loadAndSortEntries();
    }

    void draw(sf::RenderWindow &window)
    {
        sf::RectangleShape background(sf::Vector2f(600, 400));
        background.setFillColor(sf::Color(30, 30, 50, 220));
        background.setOutlineThickness(4.f);
        background.setOutlineColor(sf::Color(255, 215, 0));
        background.setPosition(window.getSize().x / 2 - 300, window.getSize().y / 2 - 200);
        window.draw(background);

        sf::Text title("TOP PLAYERS", font, 48);
        title.setFillColor(sf::Color(255, 215, 0));
        title.setPosition(window.getSize().x / 2 - title.getLocalBounds().width / 2,
                          window.getSize().y / 2 - 180);
        window.draw(title);

        sf::Text rankHeader("RANK", font, 28);
        rankHeader.setPosition(window.getSize().x / 2 - 250, window.getSize().y / 2 - 120);
        window.draw(rankHeader);

        sf::Text nameHeader("PLAYER", font, 28);
        nameHeader.setPosition(window.getSize().x / 2 - 100, window.getSize().y / 2 - 120);
        window.draw(nameHeader);

        sf::Text diamondHeader("DIAMONDS", font, 28);
        diamondHeader.setPosition(window.getSize().x / 2 + 150, window.getSize().y / 2 - 120);
        window.draw(diamondHeader);

        for (int i = 0; i < MAX_ENTRIES; i++)
        {
            sf::Text rank(std::to_string(i + 1) + ".", font, 24);
            rank.setPosition(window.getSize().x / 2 - 250, window.getSize().y / 2 - 80 + i * 50);
            window.draw(rank);

            sf::Text name(entries[i].username, font, 24);
            name.setPosition(window.getSize().x / 2 - 100, window.getSize().y / 2 - 80 + i * 50);
            window.draw(name);

            sf::Text diamonds(std::to_string(entries[i].diamonds), font, 24);
            diamonds.setPosition(window.getSize().x / 2 + 150, window.getSize().y / 2 - 80 + i * 50);
            window.draw(diamonds);
        }

        Button closeButton(font, "CLOSE", 28,
                           sf::Color(255, 100, 100),
                           sf::Color(255, 150, 150),
                           sf::Color(200, 50, 50));
        closeButton.setPosition(window.getSize().x / 2 - closeButton.getBounds().width / 2,
                                window.getSize().y / 2 + 150);
        closeButton.draw(window);
    }

    bool handleInput(const sf::Event &event, const sf::Vector2f &mousePos, const sf::RenderWindow &window)
    {
        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left)
        {
            if (mousePos.x >= window.getSize().x / 2 - 50 &&
                mousePos.x <= window.getSize().x / 2 + 50 &&
                mousePos.y >= window.getSize().y / 2 + 150 &&
                mousePos.y <= window.getSize().y / 2 + 190)
            {
                return true;
            }
        }
        return false;
    }
};

// ==================== MAIN GAME CLASS ==================== //

// The core game class that manages all gameplay states (menus, battles, training).
//  It Uses composition contains game systems and encapsulation hides SFML for clean code
class MonsterPetKingdom
{
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Texture backgroundTex;
    sf::Sprite background;
    Button startButton;
    Button options[4];
    Button mainMenuButtons[6];
    sf::Clock clock;
    sf::Music bgMusic;

    bool isHomePage;
    bool isOptionsPage;
    bool isTransition;
    bool isNameInput;
    bool isMainMenu;
    bool isTrainingSelected = false;

    float transitionTimer;

    std::string playerName;
    sf::RectangleShape nameWindow;
    sf::Text nameTitle;
    sf::Text namePrompt;
    sf::Text nameDisplay;
    sf::RectangleShape nameInputBox;
    sf::Clock cursorBlinkClock;
    bool showCursor;

    sf::RectangleShape mainMenuWindow;
    sf::Text mainMenuTitle;
    sf::Text welcomeText;

    sf::Texture diamondTexture;
    sf::Sprite diamondSprite;
    sf::Text diamondText;

    PetDisplay petDisplay;
    PetSelectionWindow petSelectionWindow;
    TrainingGame trainingGame;
    BattleSelectionWindow battleSelectionWindow;
    BattleGame battleGame;
    Battle2v2Game battle2v2Game;

    UserData userData;
    Inventory inventory;

    Scoreboard scoreboard;

    int selectedOption;
    int mainMenuSelected;
    sf::Vector2f mousePos;

    void loadResources()
    {
        if (!font.loadFromFile("arial.ttf"))
        {
            std::cerr << "Error loading font!" << std::endl;
            font.loadFromFile("C:/Windows/Fonts/Arial.ttf"); // default
        }

        if (!backgroundTex.loadFromFile("background2.jpg"))
        {
            std::cerr << "Error loading background!" << std::endl;
            backgroundTex.create(window.getSize().x, window.getSize().y);
        }
        background.setTexture(backgroundTex);
        scaleBackground();

        if (!bgMusic.openFromFile("bgmusic.ogg"))
        {
            std::cerr << "Error loading music!" << std::endl;
        }
        bgMusic.setLoop(true);
        bgMusic.setVolume(50);
        bgMusic.play();
    }

    void scaleBackground()
    {
        float scaleX = window.getSize().x / background.getLocalBounds().width;
        float scaleY = window.getSize().y / background.getLocalBounds().height;
        background.setScale(scaleX, scaleY);
    }

    void setupHomePage()
    {
        startButton = Button(font, "START GAME", 48,
                             sf::Color(255, 215, 0),
                             sf::Color(255, 255, 150),
                             sf::Color(200, 170, 0));
        centerButton(startButton, 0.6f);
    }

    void setupOptionsPage()
    {
        sf::Color normal = sf::Color(100, 200, 255);
        sf::Color hover = sf::Color(150, 230, 255);
        sf::Color active = sf::Color(50, 150, 255);

        options[0] = Button(font, "NEW GAME", 42, normal, hover, active);
        options[1] = Button(font, "CONTINUE", 42, normal, hover, active);
        options[2] = Button(font, "SCORE BOARD", 42, normal, hover, active);
        options[3] = Button(font, "EXIT GAME", 42, normal, hover, active);

        for (int i = 0; i < 4; i++)
        {
            centerButton(options[i], 0.4f + i * 0.15f);
        }

        scoreboard.setup(font, playerName, userData.getDiamonds());
    }

    void setupNameInput()
    {
        nameWindow.setSize(sf::Vector2f(650, 300));
        nameWindow.setFillColor(sf::Color(30, 30, 40, 220));
        nameWindow.setOutlineThickness(4.f);
        nameWindow.setOutlineColor(sf::Color(255, 215, 0));

        nameTitle.setFont(font);
        nameTitle.setString("WELCOME TO MONSTER PET KINGDOM");
        nameTitle.setCharacterSize(32);
        nameTitle.setFillColor(sf::Color(255, 215, 0));

        namePrompt.setFont(font);
        namePrompt.setString("Enter your name:");
        namePrompt.setCharacterSize(28);
        namePrompt.setFillColor(sf::Color(200, 200, 255));

        nameDisplay.setFont(font);
        nameDisplay.setCharacterSize(32);
        nameDisplay.setFillColor(sf::Color::White);

        nameInputBox.setSize(sf::Vector2f(400, 40));
        nameInputBox.setFillColor(sf::Color(50, 50, 70));
        nameInputBox.setOutlineThickness(2.f);
        nameInputBox.setOutlineColor(sf::Color(150, 150, 180));

        showCursor = true;
    }

    void setupMainMenu()
    {
        mainMenuWindow.setSize(sf::Vector2f(700, 600));
        mainMenuWindow.setFillColor(sf::Color(30, 30, 40, 220));
        mainMenuWindow.setOutlineThickness(4.f);
        mainMenuWindow.setOutlineColor(sf::Color(255, 215, 0));

        mainMenuTitle.setFont(font);
        mainMenuTitle.setString("MAIN MENU");
        mainMenuTitle.setCharacterSize(48);
        mainMenuTitle.setFillColor(sf::Color(255, 215, 0));

        welcomeText.setFont(font);
        welcomeText.setCharacterSize(24);
        welcomeText.setFillColor(sf::Color(255, 215, 0));
        welcomeText.setString("Welcome, " + playerName + "!");

        if (!diamondTexture.loadFromFile("diamond.png"))
        {
            sf::Image diamondImg;
            diamondImg.create(32, 32, sf::Color::Transparent);
            for (int y = 0; y < 16; y++)
            {
                for (int x = 15 - y; x <= 15 + y; x++)
                {
                    diamondImg.setPixel(x, y, sf::Color(100, 255, 255));
                }
            }
            for (int y = 16; y < 32; y++)
            {
                for (int x = y - 16; x <= 47 - y; x++)
                {
                    diamondImg.setPixel(x, y, sf::Color(100, 255, 255));
                }
            }
            diamondTexture.loadFromImage(diamondImg);
        }

        diamondSprite.setTexture(diamondTexture);
        diamondSprite.setScale(0.8f, 0.8f);

        diamondText.setFont(font);
        diamondText.setString(std::to_string(userData.getDiamonds()));
        diamondText.setCharacterSize(32);
        diamondText.setFillColor(sf::Color(32, 178, 170));
        diamondText.setOutlineThickness(2.f);
        diamondText.setOutlineColor(sf::Color::Black);

        diamondSprite.setPosition(20, 20);
        diamondText.setPosition(
            diamondSprite.getPosition().x + diamondSprite.getGlobalBounds().width + 10,
            20);

        mainMenuButtons[0] = Button(font, "BATTLE", 36,
                                    sf::Color(255, 100, 100), sf::Color(255, 150, 150), sf::Color(200, 50, 50));

        mainMenuButtons[1] = Button(font, "GUILD WAR", 36,
                                    sf::Color(100, 100, 255), sf::Color(150, 150, 255), sf::Color(50, 50, 200));

        mainMenuButtons[2] = Button(font, "TRAINING", 36,
                                    sf::Color(100, 255, 100), sf::Color(150, 255, 150), sf::Color(50, 200, 50));

        mainMenuButtons[3] = Button(font, "INVENTORY", 36,
                                    sf::Color(255, 255, 100), sf::Color(255, 255, 150), sf::Color(200, 200, 50));

        mainMenuButtons[4] = Button(font, "SHOW PETS", 36,
                                    sf::Color(200, 100, 255), sf::Color(220, 150, 255), sf::Color(160, 50, 200));

        mainMenuButtons[5] = Button(font, "EXIT", 36,
                                    sf::Color(150, 150, 150), sf::Color(200, 200, 200), sf::Color(100, 100, 100));

        float startY = 220;
        float spacing = 75;
        for (int i = 0; i < 6; i++)
        {
            mainMenuButtons[i].setPosition(
                window.getSize().x / 2 - mainMenuButtons[i].getBounds().width / 2,
                startY + i * spacing);
        }

        mainMenuSelected = -1;

        petDisplay.setup(font);
        petSelectionWindow.setup(font);
        inventory.setup(font, &userData);
        battleSelectionWindow.setup(font);

        Pet *pets[4] = {
            new Dragon(),
            new Phoenix(),
            new Griffin(),
            new Unicorn()};
        for (int i = 0; i < 4; i++)
        {
            pets[i]->setup(font);
        }
    }

    void centerButton(Button &button, float verticalRatio)
    {
        button.setPosition(
            window.getSize().x / 2 - button.getBounds().width / 2,
            window.getSize().y * verticalRatio - button.getBounds().height / 2);
    }

    void handleHomePageInput(const sf::Event &event)
    {
        if (event.type == sf::Event::MouseMoved)
        {
            mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
            startButton.update(mousePos);
        }
        else if (event.type == sf::Event::MouseButtonPressed &&
                 event.mouseButton.button == sf::Mouse::Left)
        {
            if (startButton.contains(mousePos))
            {
                isHomePage = false;
                isTransition = true;
                transitionTimer = 1.0f;
            }
        }
    }

    void handleOptionsPageInput(const sf::Event &event)
    {
        if (event.type == sf::Event::MouseMoved)
        {
            mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));

            for (int i = 0; i < 4; i++)
            {
                options[i].setActive(false);
                options[i].update(mousePos);
                if (options[i].contains(mousePos))
                {
                    selectedOption = i;
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed &&
                 event.mouseButton.button == sf::Mouse::Left)
        {
            if (selectedOption >= 0 && selectedOption < 4)
            {
                executeSelectedOption();
            }
        }
    }

    void handleNameInput(const sf::Event &event)
    {
        if (event.type == sf::Event::TextEntered)
        {
            if (event.text.unicode == '\b')
            {
                std::cerr << "   No name input !!!";
            }
            else if (event.text.unicode < 128 && event.text.unicode != '\r' && playerName.size() < 16)
            {
                playerName += static_cast<char>(event.text.unicode);
            }
            cursorBlinkClock.restart();
            showCursor = true;
        }
        else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
        {
            if (!playerName.empty())
            {
                isNameInput = false;
                isMainMenu = true;
                userData.loadUserData(playerName);
                setupMainMenu();
            }
        }
    }

    void handleMainMenuInput(const sf::Event &event)
    {
        if (event.type == sf::Event::MouseMoved)
        {
            mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));

            for (int i = 0; i < 6; i++)
            {
                mainMenuButtons[i].setActive(false);
                mainMenuButtons[i].update(mousePos);
                if (mainMenuButtons[i].contains(mousePos))
                {
                    mainMenuSelected = i;
                }
            }

            if (battle2v2Game.isOpen())
            {
                battle2v2Game.update();
            }
            else if (trainingGame.isOpen())
            {
                trainingGame.handleInput(event, mousePos);
                if (!trainingGame.isOpen())
                {
                    isMainMenu = true;
                    userData.updateUserData();
                    if (petDisplay.isOpen())
                    {
                        petDisplay.setup(font);
                        petDisplay.open();
                    }
                }
            }
            else if (inventory.isOpen())
            {
                inventory.update(mousePos);
                if (!inventory.isOpen())
                {
                    updateDiamondDisplay();
                    scoreboard.setup(font, playerName, userData.getDiamonds());
                }
            }
            else if (petDisplay.isOpen())
            {
                petDisplay.update(mousePos);
            }
            else if (petSelectionWindow.isOpen())
            {
                petSelectionWindow.update(mousePos);
            }
            else if (battleSelectionWindow.isOpen())
            {
                battleSelectionWindow.update(mousePos);
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed &&
                 event.mouseButton.button == sf::Mouse::Left)
        {

            if (battle2v2Game.isOpen())
            {
                battle2v2Game.handleInput(event, mousePos);
                if (!battle2v2Game.isOpen())
                {
                    isMainMenu = true;
                }
            }
            else if (trainingGame.isOpen())
            {
                trainingGame.handleInput(event, mousePos);
                if (!trainingGame.isOpen())
                {
                    isMainMenu = true;
                }
            }
            else if (inventory.isOpen())
            {
                inventory.handleInput(event, mousePos);
            }
            else if (petDisplay.isOpen())
            {
                petDisplay.handleInput(event, mousePos);
            }
            else if (battleSelectionWindow.isOpen())
            {
                battleSelectionWindow.handleInput(event, mousePos);

                if (!battleSelectionWindow.isOpen())
                {
                    int selected = battleSelectionWindow.getSelectedBattle();
                    if (selected == 0)
                    {
                        petSelectionWindow.open();
                    }
                    else if (selected == 1)
                    {
                        static int petsSelected = 0;
                        static Pet *selectedPets[2] = {nullptr, nullptr};

                        if (petsSelected < 2)
                        {
                            petSelectionWindow.open();
                        }
                    }
                }
            }
            else if (petSelectionWindow.isOpen())
            {
                petSelectionWindow.handleInput(event, mousePos);

                if (!petSelectionWindow.isOpen() && petSelectionWindow.getSelectedPet())
                {
                    if (isTrainingSelected)
                    {
                        trainingGame.setup(font, petSelectionWindow.getSelectedPet());
                        trainingGame.open(petSelectionWindow.getSelectedPet());
                        isTrainingSelected = false;
                    }
                    if (battleSelectionWindow.getSelectedBattle() == 0)
                    {
                        Pet *enemy;
                        int enemyType = rand() % 4;
                        switch (enemyType)
                        {
                        case 0:
                            enemy = new Dragon();
                            break;
                        case 1:
                            enemy = new Phoenix();
                            break;
                        case 2:
                            enemy = new Griffin();
                            break;
                        case 3:
                            enemy = new Unicorn();
                            break;
                        }
                        enemy->setup(font);

                        battleGame.setup(font, petSelectionWindow.getSelectedPet(), enemy);
                        battleGame.open();
                    }
                    else if (battleSelectionWindow.getSelectedBattle() == 1)
                    {
                        static int petsSelected = 0;
                        static Pet *selectedPets[2] = {nullptr, nullptr};

                        selectedPets[petsSelected] = petSelectionWindow.getSelectedPet();
                        petsSelected++;

                        if (petsSelected < 2)
                        {
                            petSelectionWindow.open();
                        }
                        else
                        {
                            Pet *enemy1;
                            Pet *enemy2;
                            int enemyType1 = rand() % 4;
                            int enemyType2 = rand() % 4;

                            switch (enemyType1)
                            {
                            case 0:
                                enemy1 = new Dragon();
                                break;
                            case 1:
                                enemy1 = new Phoenix();
                                break;
                            case 2:
                                enemy1 = new Griffin();
                                break;
                            case 3:
                                enemy1 = new Unicorn();
                                break;
                            }
                            enemy1->setup(font);

                            switch (enemyType2)
                            {
                            case 0:
                                enemy2 = new Dragon();
                                break;
                            case 1:
                                enemy2 = new Phoenix();
                                break;
                            case 2:
                                enemy2 = new Griffin();
                                break;
                            case 3:
                                enemy2 = new Unicorn();
                                break;
                            }
                            enemy2->setup(font);

                            battle2v2Game.setup(font, selectedPets[0], selectedPets[1], enemy1, enemy2);
                            battle2v2Game.open();

                            petsSelected = 0;
                            selectedPets[0] = selectedPets[1] = nullptr;
                        }
                    }
                    else
                    {
                        trainingGame.setup(font, petSelectionWindow.getSelectedPet());
                        trainingGame.open(petSelectionWindow.getSelectedPet());
                    }
                }
            }

            else if (mainMenuSelected >= 0 && mainMenuSelected < 6)
            {
                handleMainMenuSelection();
            }
        }
    }

    void executeSelectedOption()
    {
        if (selectedOption == 0) // newgame
        {
            isOptionsPage = false;
            isNameInput = true;
            playerName.clear();
            cursorBlinkClock.restart();
            setupNameInput();
        }
        else if (selectedOption == 1) // continue
        {
            isOptionsPage = false;
            isNameInput = true;
            playerName.clear();
            cursorBlinkClock.restart();
            setupNameInput();
        }
        else if (selectedOption == 2) // scoreboard
        {
            showScoreboard();
        }
        else if (selectedOption == 3) // exit
        {
            window.close();
        }
    }

    void handleMainMenuSelection()
    {
        switch (mainMenuSelected)
        {
        case 0: // battle
            battleSelectionWindow.open();
            userData.addDiamonds(50);
            diamondText.setString(std::to_string(userData.getDiamonds()));
            userData.updateUserData();
            diamondText.setString(std::to_string(userData.getDiamonds()));
            scoreboard.setup(font, playerName, userData.getDiamonds());
            break;
        case 1: // guildwar
            break;
        case 2: // Training
            isTrainingSelected = true;
            petSelectionWindow.open();
            break;
        case 3: // inventory
            inventory.open();
            break;
        case 4: // showpets
            petDisplay.open();
            break;
        case 5: // exit
            window.close();
            break;
        }
    }

    void updateDiamondDisplay()
    {
        diamondText.setString(std::to_string(userData.getDiamonds()));
    }

public:
    MonsterPetKingdom() : window(sf::VideoMode(1280, 720), "Monster Pet Kingdom", sf::Style::Close),
                          isHomePage(true),
                          isOptionsPage(false),
                          isTransition(false),
                          isNameInput(false),
                          isMainMenu(false),
                          transitionTimer(0.0f),
                          selectedOption(-1),
                          mainMenuSelected(-1),
                          showCursor(true)
    {
        loadResources();
        setupHomePage();
    }

    void run()
    {
        sf::Clock deltaClock;
        while (window.isOpen())
        {
            float deltaTime = deltaClock.restart().asSeconds();
            handleEvents();
            update(deltaTime);
            render();
        }
    }

    void showScoreboard()
    {
        bool showingScoreboard = true;
        while (showingScoreboard && window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                    showingScoreboard = false;
                }

                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                if (scoreboard.handleInput(event, mousePos, window))
                {
                    showingScoreboard = false;
                }
            }

            window.clear();
            window.draw(background);
            scoreboard.draw(window);
            window.display();
        }
    }

private:
    void handleEvents()
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            if (battle2v2Game.isOpen())
            {
                battle2v2Game.handleInput(event, mousePos);
                continue;
            }
            else if (battleGame.isOpen())
            {
                battleGame.handleInput(event, mousePos);
                continue;
            }

            if (trainingGame.isOpen())
            {
                trainingGame.handleInput(event, mousePos);
                continue;
            }
            if (inventory.isOpen())
            {
                inventory.handleInput(event, mousePos);
                if (!inventory.isOpen())
                {
                    updateDiamondDisplay();
                }
                continue;
            }
            else if (isHomePage)
            {
                handleHomePageInput(event);
            }
            else if (isOptionsPage)
            {
                handleOptionsPageInput(event);
            }
            else if (isNameInput)
            {
                handleNameInput(event);
            }
            else if (isMainMenu)
            {
                handleMainMenuInput(event);
            }
        }
    }

    void update(float deltaTime)
    {
        mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        if (trainingGame.isOpen())
        {
            trainingGame.update(mousePos);
        }

        if (battle2v2Game.isOpen())
        {
            battle2v2Game.update();
            return;
        }
        else if (battleGame.isOpen())
        {
            battleGame.update();
            return;
        }

        else if (inventory.isOpen())
        {
            inventory.update(mousePos);
        }
        else if (isHomePage)
        {
            startButton.update(mousePos);
        }
        else if (isTransition)
        {
            transitionTimer -= deltaTime;
            if (transitionTimer <= 0.0f)
            {
                isTransition = false;
                isOptionsPage = true;
                setupOptionsPage();
            }
        }
        else if (isOptionsPage)
        {
            for (int i = 0; i < 4; i++)
            {
                options[i].update(mousePos);
            }
        }
        else if (isNameInput)
        {
            if (cursorBlinkClock.getElapsedTime().asSeconds() > 0.5f)
            {
                showCursor = !showCursor;
                cursorBlinkClock.restart();
            }
        }
        else if (isMainMenu)
        {
            for (int i = 0; i < 6; i++)
            {
                mainMenuButtons[i].update(mousePos);
            }
        }
    }

    void render()
    {
        window.clear();
        window.draw(background);

        if (isMainMenu)
        {
            mainMenuWindow.setPosition(window.getSize().x / 2 - 350, window.getSize().y / 2 - 300);
            mainMenuTitle.setPosition(window.getSize().x / 2 - mainMenuTitle.getLocalBounds().width / 2,
                                      window.getSize().y / 2 - 260);
            welcomeText.setPosition(window.getSize().x / 2 - welcomeText.getLocalBounds().width / 2,
                                    window.getSize().y / 2 - 200);

            window.draw(mainMenuWindow);
            window.draw(mainMenuTitle);
            window.draw(welcomeText);
            window.draw(diamondSprite);
            window.draw(diamondText);

            for (int i = 0; i < 6; i++)
            {
                mainMenuButtons[i].draw(window);
            }
        }

        if (battle2v2Game.isOpen())
        {
            battle2v2Game.draw(window);
        }
        else if (battleGame.isOpen())
        {
            battleGame.draw(window);
        }
        else if (trainingGame.isOpen())
        {
            trainingGame.draw(window);
        }
        else if (petDisplay.isOpen())
        {
            petDisplay.draw(window);
        }
        else if (petSelectionWindow.isOpen())
        {
            petSelectionWindow.draw(window);
        }
        else if (inventory.isOpen())
        {
            inventory.draw(window);
        }
        else if (battleSelectionWindow.isOpen())
        {
            battleSelectionWindow.draw(window);
        }

        else if (isHomePage)
        {
            sf::Text title("MONSTER PET KINGDOM", font, 72);
            title.setFillColor(sf::Color(255, 215, 0));
            title.setOutlineColor(sf::Color::Black);
            title.setOutlineThickness(2.0f);
            title.setPosition(window.getSize().x / 2 - title.getLocalBounds().width / 2,
                              window.getSize().y * 0.25f);
            window.draw(title);

            startButton.draw(window);
        }
        else if (isTransition)
        {
            sf::RectangleShape loadingBox(sf::Vector2f(300, 80));
            loadingBox.setFillColor(sf::Color(50, 50, 50, 200));
            loadingBox.setOutlineThickness(2.f);
            loadingBox.setOutlineColor(sf::Color(255, 215, 0));
            loadingBox.setPosition(window.getSize().x / 2 - 150, window.getSize().y / 2 - 40);

            sf::Text loading("LOADING...", font, 36);
            loading.setFillColor(sf::Color(255, 215, 0));
            loading.setPosition(window.getSize().x / 2 - loading.getLocalBounds().width / 2,
                                window.getSize().y / 2 - loading.getLocalBounds().height / 2);

            window.draw(loadingBox);
            window.draw(loading);
        }
        else if (isOptionsPage)
        {
            sf::Text title("MONSTER PET KINGDOM", font, 72);
            title.setFillColor(sf::Color(255, 215, 0));
            title.setOutlineColor(sf::Color::Black);
            title.setOutlineThickness(2.0f);
            title.setPosition(window.getSize().x / 2 - title.getLocalBounds().width / 2,
                              window.getSize().y * 0.15f);
            window.draw(title);

            for (int i = 0; i < 4; i++)
            {
                options[i].draw(window);
            }
        }
        else if (isNameInput)
        {
            nameWindow.setPosition(window.getSize().x / 2 - 325, window.getSize().y / 2 - 150);
            nameTitle.setPosition(window.getSize().x / 2 - nameTitle.getLocalBounds().width / 2,
                                  window.getSize().y / 2 - 120);
            namePrompt.setPosition(window.getSize().x / 2 - namePrompt.getLocalBounds().width / 2,
                                   window.getSize().y / 2 - 50);
            nameInputBox.setPosition(window.getSize().x / 2 - 200, window.getSize().y / 2);
            nameDisplay.setString(playerName + (showCursor ? "_" : ""));
            nameDisplay.setPosition(window.getSize().x / 2 - nameDisplay.getLocalBounds().width / 2,
                                    window.getSize().y / 2 + 5);

            window.draw(nameWindow);
            window.draw(nameTitle);
            window.draw(namePrompt);
            window.draw(nameInputBox);
            window.draw(nameDisplay);
        }

        window.display();
    }
};

// ----------- Main fxn -------------//

int main()
{
    MonsterPetKingdom game;
    game.run();
    return 0;
}

//handle input-  checks what keys or buttons the player is pressing right now
//handle event-  deals with like mouse clicks or window closing
//update- updates the game logic like moving things or checking for collisions
//draw- puts everything on the screen so the player can see it
//setup- gets everything ready like loading images and setting positions


