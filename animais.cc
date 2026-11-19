#include "animais.hh"
#include "ambiente.hh"

Genes genRandomGenes(std::mt19937& gen) {

    std::normal_distribution<double> normalDist(0.5, 0.15); 
    std::uniform_int_distribution<int> genderDist(0, 1);
    std::normal_distribution<double> tempDist(0.0, 33.0);

    Genes g;
    g.gender = genderDist(gen);
    
    g.size = std::max(0.1, normalDist(gen));
    g.physicCapacity = std::max(0.1, normalDist(gen));
    g.agingEffect = std::max(0.01, normalDist(gen));

    double baseTemp = tempDist(gen);
    g.idealTemp.minTemp = baseTemp - 5.0;
    g.idealTemp.maxTemp = baseTemp + 5.0;
    g.tempEffect = std::max(0.05, normalDist(gen));

    g.minHumid = std::clamp(normalDist(gen), 0.0, 1.0);
    g.humidEffect = std::max(0.05, normalDist(gen));
    
    g.metabolism = std::max(0.1, normalDist(gen));
    g.foodNeed = std::max(0.1, normalDist(gen));
    g.foodDigestion = std::max(0.1, normalDist(gen));

    g.mateAwareness = std::max(0.1, normalDist(gen));
    g.predAwareness = std::max(0.1, normalDist(gen));
    g.preyAwareness = std::max(0.1, normalDist(gen));

    g.foodOrientation = std::max(0.1, normalDist(gen));
    g.tempOrientation = std::max(0.1, normalDist(gen));
    g.humidOrientation = std::max(0.1, normalDist(gen));
    g.breedOrientation = std::max(0.1, normalDist(gen));

    g.matureAge = std::max(1.0, normalDist(gen) * 10);
    g.fertility = std::max(0.1, normalDist(gen));

    return g;
}

Animal::Animal(uint id, int initX, int initY, Genes genes)
    : ID(id), age(0), energyReservatory(100.0), vitalEnergy(100.0), tileX(initX), tileY(initY), DNA(genes) {}

Frog::Frog(unsigned int id, float startX, float startY, Genes genes)
    : Animal(id, startX, startY, genes) {}

Snake::Snake(unsigned int id, float startX, float startY, Genes genes)
    : Animal(id, startX, startY, genes) {}

void Animal::breathe(generator& world) {
    square& currentTile = world.getSquare(tileX, tileY);
    double currentTemp = currentTile.getTemperature();
    double currentHumid = currentTile.getHumidity();

    double trueMetabolism = DNA.metabolism * age * DNA.agingEffect;

    if(!DNA.idealTemp.isIdealTemp(currentTemp)) {
        double tempDiff = std::max((std::abs(currentTemp - DNA.idealTemp.minTemp)), 
                                    (std::abs(DNA.idealTemp.maxTemp - currentTemp)));
        trueMetabolism += (tempDiff * DNA.tempEffect * 0.1); 
    }

    if(currentHumid < DNA.minHumid) {
        double humidDiff = DNA.minHumid - currentHumid;
        trueMetabolism += (humidDiff * DNA.humidEffect * 5.0);
    }

    energyReservatory -= trueMetabolism;
    if (energyReservatory < 0) {
        vitalEnergy += energyReservatory; 
        energyReservatory = 0;
    }

    if (energyReservatory > 100.0) energyReservatory = 100.0;
    if (vitalEnergy > 100.0) vitalEnergy = 100.0;
}

void Frog::eat(generator& world) {
    square& currentTile = world.getSquare(tileX, tileY);
    double disponibleFood = currentTile.getFood();

    if(disponibleFood > (DNA.foodNeed * 0.1)) {

        if(currentTile.getFrogNum() > currentTile.getSatPrey()) {

            energyReservatory += 15.0;

        } else {

            energyReservatory += 30.0; 
        }
    }
}

void Frog::move(generator& world) {

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::normal_distribution<double> noise(0.0, 0.05);

    struct Decision {

        OrientationIntent intent;
        double value;

    };

    struct TileInfo {

        square* tile;
        int x;
        int y;
    };

    std::vector<TileInfo> adjacentTiles;

    // todas as possibilidades de movimento
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0,  1, -1, 1, -1, 0, 1};

    for(int i = 0; i < 8; i++){

        int mx = tileX + dx[i];
        int my = tileY + dy[i];

        if(mx < GRID_SIZE && mx >= 0 && my < GRID_SIZE && my >= 0) {
            adjacentTiles.push_back({ &world.getSquare(mx, my), mx, my} );
        }
    }

    if(energyReservatory < (DNA.metabolism * age * DNA.agingEffect)) {

        double trueFoodOrientation = (DNA.foodOrientation * 1.15);
    }

    std::vector<Decision> options = {

    { OrientationIntent::Food,        DNA.foodOrientation + noise(gen) },
    { OrientationIntent::Breed,       DNA.breedOrientation + noise(gen) },
    { OrientationIntent::Humidity,    DNA.humidOrientation + noise(gen) },
    { OrientationIntent::Temperature, DNA.tempOrientation + noise(gen) }

    };

    auto bestDecision = std::max_element(options.begin(), options.end(), 

    [](const Decision& a, const Decision& b) {
        return a.value < b.value;
    });

    OrientationIntent chosenOrientation = bestDecision->intent;

    std::function<double(square*)> evaluateTile;

    switch (chosenOrientation) {
        case OrientationIntent::Food:

            evaluateTile = [](square* s) { return s->getFood(); }; 
            break;

        case OrientationIntent::Breed:
            evaluateTile = [&](square* s) { 

                for (Frog* otherFrog : s->getFrogs()) {

                    if (otherFrog->DNA.gender != this->DNA.gender) {
                        return 1.0; 
                    }
                }

                return 0.0; 
            }; 
            break;

        case OrientationIntent::Humidity:

            evaluateTile = [](square* s) { return s->getHumidity(); }; 
            break;

        case OrientationIntent::Temperature:

            evaluateTile = [&](square* s) { 
                
                double idealMedia = (DNA.idealTemp.minTemp + DNA.idealTemp.maxTemp) / 2.0;
                return -std::abs(s->getTemperature() - idealMedia); 

            }; 
            break;
    }

    struct TileScore {

        TileInfo info;
        double score;
    };

    std::vector<TileScore> tileScores;

    for (auto& info : adjacentTiles) {
        TileScore ts;
        ts.info = info;
        ts.score = evaluateTile(info.tile);
        
        tileScores.push_back(ts);
    }

    std::sort(tileScores.begin(), tileScores.end(), [](const TileScore& a, const TileScore& b) {
        return a.score > b.score;
    });

    std::vector<double> weights = { 0.75, 0.15, 0.1 }; 
    
    int numChoices = std::min(3, static_cast<int>(tileScores.size()));
    weights.resize(numChoices);

    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    int chosenIndex = dist(gen);

    TileInfo targetTile = tileScores[chosenIndex].info;

    this->tileX = targetTile.x;
    this->tileY = targetTile.y;
}

Animal* Frog::reproduce(generator& world) {

    square& currentTile = world.getSquare(this->tileX, this->tileY);

    for (Frog* otherFrog : currentTile.getFrogs()) {
        if (otherFrog != this && otherFrog->DNA.gender != this->DNA.gender) {
            
            this->reproduce(world, *otherFrog);
            break;
        }
    }

    return nullptr;
}

Frog* Frog::reproduce(generator& world, const Frog& partner) {
    
    if (this->DNA.gender == partner.DNA.gender) {
        return nullptr;
    }

    if (this->age < this->DNA.matureAge || partner.age < partner.DNA.matureAge) {
        return nullptr;
    }

    double energyCost = 25.0;
    if (this->energyReservatory < energyCost) {
        return nullptr;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> chanceDist(0.0, 1.0);
    
    double avgFertility = (this->DNA.fertility + partner.DNA.fertility) / 2.0;
    if (chanceDist(gen) > avgFertility) {
        return nullptr;
    }

    std::normal_distribution<double> mutation(0.0, 0.05);
    std::uniform_int_distribution<int> genderDist(0, 1);

    auto mixGenes = [&](double a, double b) {
        return std::max(0.01, ((a + b) / 2.0) + mutation(gen));
    };

    Genes childDNA;
    childDNA.gender = genderDist(gen);
    childDNA.size = mixGenes(this->DNA.size, partner.DNA.size);
    childDNA.physicCapacity = mixGenes(this->DNA.physicCapacity, partner.DNA.physicCapacity);
    childDNA.agingEffect = mixGenes(this->DNA.agingEffect, partner.DNA.agingEffect);
    childDNA.tempEffect = mixGenes(this->DNA.tempEffect, partner.DNA.tempEffect);
    childDNA.humidEffect = mixGenes(this->DNA.humidEffect, partner.DNA.humidEffect);
    childDNA.metabolism = mixGenes(this->DNA.metabolism, partner.DNA.metabolism);
    childDNA.foodNeed = mixGenes(this->DNA.foodNeed, partner.DNA.foodNeed);
    childDNA.foodDigestion = mixGenes(this->DNA.foodDigestion, partner.DNA.foodDigestion);
    childDNA.matureAge = std::max(1.0, mixGenes(this->DNA.matureAge, partner.DNA.matureAge));
    childDNA.fertility = std::min(1.0, mixGenes(this->DNA.fertility, partner.DNA.fertility));
    
    childDNA.idealTemp.minTemp = (this->DNA.idealTemp.minTemp + partner.DNA.idealTemp.minTemp) / 2.0;
    childDNA.idealTemp.maxTemp = (this->DNA.idealTemp.maxTemp + partner.DNA.idealTemp.maxTemp) / 2.0;
    childDNA.minHumid = std::clamp(((this->DNA.minHumid + partner.DNA.minHumid) / 2.0) + mutation(gen), 0.0, 1.0);

    childDNA.mateAwareness = mixGenes(this->DNA.mateAwareness, partner.DNA.mateAwareness);
    childDNA.foodOrientation = mixGenes(this->DNA.foodOrientation, partner.DNA.foodOrientation);
    childDNA.tempOrientation = mixGenes(this->DNA.tempOrientation, partner.DNA.tempOrientation);
    childDNA.humidOrientation = mixGenes(this->DNA.humidOrientation, partner.DNA.humidOrientation);
    childDNA.breedOrientation = mixGenes(this->DNA.breedOrientation, partner.DNA.breedOrientation);

    this->energyReservatory -= energyCost;

    static uint nextID = 10000; 
    Frog* childFrog = new Frog(nextID++, this->tileX, this->tileY, childDNA);

    world.getSquare(this->tileX, this->tileY).enterFrog(childFrog);

    return childFrog;
}

void Snake::move(generator& world) {

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<double> noise(0.0, 0.05);

    struct Decision {
        OrientationIntent intent;
        double value;
    };

    struct TileInfo {
        square* tile;
        int x;
        int y;
    };

    std::vector<TileInfo> adjacentTiles;

    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0,  1, -1, 1, -1, 0, 1};

    for(int i = 0; i < 8; i++){
        int mx = this->tileX + dx[i];
        int my = this->tileY + dy[i];

        if(mx < GRID_SIZE && mx >= 0 && my < GRID_SIZE && my >= 0) {

            adjacentTiles.push_back({ &world.getSquare(mx, my), mx, my} );
        }
    }

    if(this->energyReservatory < (this->DNA.metabolism * this->age * this->DNA.agingEffect)) {
        double trueFoodOrientation = (this->DNA.foodOrientation * 1.15);
    }

    std::vector<Decision> options = {

        { OrientationIntent::Food,        this->DNA.foodOrientation + noise(gen) },
        { OrientationIntent::Breed,       this->DNA.breedOrientation + noise(gen) },
        { OrientationIntent::Humidity,    this->DNA.humidOrientation + noise(gen) },
        { OrientationIntent::Temperature, this->DNA.tempOrientation + noise(gen) }

    };

    auto bestDecision = std::max_element(options.begin(), options.end(), 
    [](const Decision& a, const Decision& b) {
        return a.value < b.value;
    });

    OrientationIntent chosenOrientation = bestDecision->intent;

    std::function<double(square*)> evaluateTile;

    switch (chosenOrientation) {
        case OrientationIntent::Food:

            evaluateTile = [](square* s) { return static_cast<double>(s->getFrogNum()); }; 
            break;

        case OrientationIntent::Breed:

            evaluateTile = [&](square* s) { 
                for (Snake* otherSnake : s->getSnakes()) {
                    if (otherSnake->getDNA().gender != this->DNA.gender) {
                        return 1.0; 
                    }
                }
                return 0.0; 
            }; 
            break;

        case OrientationIntent::Humidity:
            evaluateTile = [](square* s) { return s->getHumidity(); }; 
            break;

        case OrientationIntent::Temperature:
            evaluateTile = [&](square* s) { 
                double idealMedia = (this->DNA.idealTemp.minTemp + this->DNA.idealTemp.maxTemp) / 2.0;
                return -std::abs(s->getTemperature() - idealMedia); 
            }; 
            break;
    }

    struct TileScore {
        TileInfo info;
        double score;
    };

    std::vector<TileScore> tileScores;

    for (auto& info : adjacentTiles) {
        TileScore ts;
        ts.info = info;
        ts.score = evaluateTile(info.tile);
        tileScores.push_back(ts);
    }

    std::sort(tileScores.begin(), tileScores.end(), [](const TileScore& a, const TileScore& b) {
        return a.score > b.score;
    });

    std::vector<double> weights = { 0.75, 0.15, 0.1 }; 
    int numChoices = std::min(3, static_cast<int>(tileScores.size()));
    weights.resize(numChoices);

    std::discrete_distribution<int> dist(weights.begin(), weights.end());
    int chosenIndex = dist(gen);

    TileInfo targetTile = tileScores[chosenIndex].info;

    world.getSquare(this->tileX, this->tileY).leaveSnake(this);
    this->tileX = targetTile.x;
    this->tileY = targetTile.y;
    world.getSquare(this->tileX, this->tileY).enterSnake(this);
}

void Snake::eat(generator& world) {

    square& currentTile = world.getSquare(this->tileX, this->tileY);
    std::vector<Frog*>& preys = currentTile.getFrogs();

    if (preys.empty()) {
        return;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> chanceDist(0.0, 1.0);

    std::vector<Frog*> currentPreys = preys;

    for (Frog* presa : currentPreys) {
 
        if (!presa->isAlive()) continue;

        double snakePhysic = this->DNA.physicCapacity;
        double frogPhysic = presa->getDNA().physicCapacity;
        
        double successChance = snakePhysic / (snakePhysic + frogPhysic * 1.5); 

        if (chanceDist(gen) < successChance) {

            this->energyReservatory += 40.0;

            presa->kill(); 
            currentTile.leaveFrog(presa);
            break; 
        }
    }
}

Animal* Snake::reproduce(generator& world) {
    square& currentTile = world.getSquare(this->tileX, this->tileY);
    for (Snake* otherSnake : currentTile.getSnakes()) {
        if (otherSnake != this && otherSnake->getDNA().gender != this->DNA.gender) {

            return this->reproduce(world, *otherSnake); 
        }
    }
    return nullptr;
}

Snake* Snake::reproduce(generator& world, const Snake& partner) {
    
    if (this->DNA.gender == partner.DNA.gender) {
        return nullptr;
    }

    if (this->age < this->DNA.matureAge || partner.age < partner.DNA.matureAge) {
        return nullptr;
    }

    double energyCost = 25.0;
    if (this->energyReservatory < energyCost) {
        return nullptr;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<double> chanceDist(0.0, 1.0);
    

    double avgFertility = (this->DNA.fertility + partner.getDNA().fertility) / 2.0;
    if (chanceDist(gen) > avgFertility) {
        return nullptr;
    }

    std::normal_distribution<double> mutation(0.0, 0.05);
    std::uniform_int_distribution<int> genderDist(0, 1);


    auto mixGenes = [&](double a, double b) {
        return std::max(0.01, ((a + b) / 2.0) + mutation(gen));
    };


    Genes childDNA;
    childDNA.gender = genderDist(gen);
    childDNA.size = mixGenes(this->DNA.size, partner.getDNA().size);
    childDNA.physicCapacity = mixGenes(this->DNA.physicCapacity, partner.getDNA().physicCapacity);
    childDNA.agingEffect = mixGenes(this->DNA.agingEffect, partner.getDNA().agingEffect);
    childDNA.tempEffect = mixGenes(this->DNA.tempEffect, partner.getDNA().tempEffect);
    childDNA.humidEffect = mixGenes(this->DNA.humidEffect, partner.getDNA().humidEffect);
    childDNA.metabolism = mixGenes(this->DNA.metabolism, partner.getDNA().metabolism);
    childDNA.foodNeed = mixGenes(this->DNA.foodNeed, partner.getDNA().foodNeed);
    childDNA.foodDigestion = mixGenes(this->DNA.foodDigestion, partner.getDNA().foodDigestion);
    childDNA.matureAge = std::max(1.0, mixGenes(this->DNA.matureAge, partner.getDNA().matureAge));
    childDNA.fertility = std::min(1.0, mixGenes(this->DNA.fertility, partner.getDNA().fertility));
    
    childDNA.idealTemp.minTemp = (this->DNA.idealTemp.minTemp + partner.getDNA().idealTemp.minTemp) / 2.0;
    childDNA.idealTemp.maxTemp = (this->DNA.idealTemp.maxTemp + partner.getDNA().idealTemp.maxTemp) / 2.0;
    childDNA.minHumid = std::clamp(((this->DNA.minHumid + partner.getDNA().minHumid) / 2.0) + mutation(gen), 0.0, 1.0);

    childDNA.mateAwareness = mixGenes(this->DNA.mateAwareness, partner.getDNA().mateAwareness);
    childDNA.foodOrientation = mixGenes(this->DNA.foodOrientation, partner.getDNA().foodOrientation);
    childDNA.tempOrientation = mixGenes(this->DNA.tempOrientation, partner.getDNA().tempOrientation);
    childDNA.humidOrientation = mixGenes(this->DNA.humidOrientation, partner.getDNA().humidOrientation);
    childDNA.breedOrientation = mixGenes(this->DNA.breedOrientation, partner.getDNA().breedOrientation);


    this->energyReservatory -= energyCost;


    static uint nextSnakeID = 20000; 
    Snake* childSnake = new Snake(nextSnakeID++, this->tileX, this->tileY, childDNA);


    world.getSquare(this->tileX, this->tileY).enterSnake(childSnake);

    return childSnake;
}

