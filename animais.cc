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
    : ID(id), tileX(initX), tileY(initY), DNA(genes), age(0), energyReservatory(100.0), vitalEnergy(100.0) {}


void Animal::breathe(generator& world) {

    square& currentTile = world.getSquare(tileX, tileY);

    double currentTemp = currentTile.getTemperature();
    double currentHumid = currentTile.getHumidity();

    double trueMetabolism = DNA.metabolism * age * DNA.agingEffect;

    if(!DNA.idealTemp.isIdealTemp(currentTemp)) {

        double tempDiff = std::max((std::abs(currentTemp - DNA.idealTemp.minTemp)), 
                                    (std::abs(DNA.idealTemp.maxTemp - currentTemp)));
        
        trueMetabolism += (tempDiff * DNA.tempEffect);

    }

    if(currentHumid < DNA.minHumid) {

        double humidDiff = DNA.minHumid - currentHumid;

        trueMetabolism += (humidDiff * DNA.humidEffect);

    }

    vitalEnergy = vitalEnergy + (energyReservatory - trueMetabolism);

}

void Frog::eat(generator& world) {

    square& currentTile = world.getSquare(tileX, tileY);

    double disponibleFood = currentTile.getFood();

    if(disponibleFood > DNA.foodNeed) {

        if(currentTile.getFrogNum() > currentTile.getSatPrey()) {

            energyReservatory += (DNA.foodNeed * DNA.foodDigestion * 0.5);

        }

        energyReservatory += (DNA.foodNeed * DNA.foodDigestion);

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

        if(mx < 3 && mx >= 0 && my < 3 && my >= 0) {
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

