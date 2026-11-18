#include <iostream>
#include <string>

#include "../src/logic/Game.h"
#include "../src/model/BotPlayer.h"
#include "../src/model/HumanPlayer.h"

using namespace std;

static int failedChecks = 0;

static void check(bool condition, const char* name) {
    if (condition) {
        cout << "[PASS] " << name << '\n';
    } else {
        cout << "[FAIL] " << name << '\n';
        ++failedChecks;
    }
}

static Card num(CardColor color, int number) {
    return Card(color, number);
}

static Card act(CardColor color, CardType type) {
    return Card(color, type);
}

static Card wild() {
    return Card(CardColor::None, CardType::Wild);
}

static void setHand(Player& player, const Card cards[], int count) {
    player.clearHand();

    for (int i = 0; i < count; ++i) {
        player.addCard(cards[i]);
    }
}

static void makeBots(DynamicArray<Player*>& seats, int count) {
    for (int i = 0; i < count; ++i) {
        seats.add(new BotPlayer("Bot_" + to_string(i + 1)));
    }
}

static void freeBots(DynamicArray<Player*>& seats) {
    for (int i = 0; i < seats.size(); ++i) {
        Player* player = nullptr;
        seats.get(i, player);
        delete player;
    }
}

static int totalCards(Round& round, int playerCount) {
    int total = round.getDeckSize() + round.getDiscardSize();

    for (int i = 0; i < playerCount; ++i) {
        total += round.getPlayerAt(i)->getHandSize();
    }

    return total;
}

static bool runRound(Game& game, bool& conserved) {
    Round* round = game.getRound();
    int count = game.getPlayerCount();
    int steps = 0;

    if (totalCards(*round, count) != 108) {
        conserved = false;
    }

    while (!round->isOver() && steps < 20000) {
        Action action;
        MoveResult result;

        if (!game.stepBot(action, result)) {
            return false;
        }

        if (totalCards(*round, count) != 108) {
            conserved = false;
        }

        ++steps;
    }

    return round->isOver();
}

static bool playFullGame(Game& game, bool& conserved) {
    if (!game.start()) {
        return false;
    }

    for (int i = 0; i < 300; ++i) {
        if (!runRound(game, conserved)) {
            return false;
        }

        game.finishRound();

        if (game.isFinished()) {
            return true;
        }

        if (!game.startNextRound()) {
            return false;
        }
    }

    return false;
}

static bool scoresConsistent(Game& game) {
    for (int i = 0; i < game.getPlayerCount(); ++i) {
        Player* player = game.getPlayerAt(i);

        if (player->getRoundScores().size() != game.getRoundsPlayed()) {
            return false;
        }

        int sum = 0;

        for (int j = 0; j < player->getRoundScores().size(); ++j) {
            int value = 0;
            player->getRoundScores().get(j, value);
            sum += value;
        }

        if (sum != player->getCurrentGameScore()) {
            return false;
        }
    }

    return true;
}

static bool winnerHasTopScore(Game& game) {
    Player* winner = game.getGameWinner();

    if (winner == nullptr) {
        return false;
    }

    for (int i = 0; i < game.getPlayerCount(); ++i) {
        if (game.getPlayerAt(i)->getCurrentGameScore() >
            winner->getCurrentGameScore()) {
            return false;
        }
    }

    return true;
}

static void testRoundScoring() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    seats.add(&a);
    seats.add(&b);
    seats.add(&c);

    a.clearHand();
    const Card handB[] = {num(CardColor::Red, 7),
                          act(CardColor::Blue, CardType::Skip)};
    const Card handC[] = {wild(), num(CardColor::Green, 0)};
    setHand(b, handB, 2);
    setHand(c, handC, 2);

    check(ScoreManager::calcRoundPoints(&a, seats) == 77 &&
          b.getHandSize() == 2 && c.getHandSize() == 2,
          "scoring: winner gets 7+20+50+0 = 77 and hands are untouched");

    int points = ScoreManager::recordRound(&a, seats);
    check(points == 77 && a.getCurrentGameScore() == 77 &&
          b.getCurrentGameScore() == 0 && c.getCurrentGameScore() == 0,
          "scoring: only the winner gains points");

    int value = -1;
    check(a.getRoundScores().size() == 1 && b.getRoundScores().size() == 1 &&
          c.getRoundScores().size() == 1 &&
          a.getRoundScores().get(0, value) && value == 77 &&
          b.getRoundScores().get(0, value) && value == 0,
          "scoring: every player gets one round entry (0 for losers)");

    const Card handA[] = {act(CardColor::Red, CardType::Reverse)};
    const Card handC2[] = {num(CardColor::Red, 1)};
    setHand(a, handA, 1);
    b.clearHand();
    setHand(c, handC2, 1);

    check(ScoreManager::recordRound(&b, seats) == 21 &&
          b.getCurrentGameScore() == 21 && a.getCurrentGameScore() == 77 &&
          a.getRoundScores().size() == 2,
          "scoring: a second round adds to the winner and keeps the history");
}

static void testGameWinnerRules() {
    HumanPlayer a("A");
    HumanPlayer b("B");
    HumanPlayer c("C");
    DynamicArray<Player*> seats;
    seats.add(&a);
    seats.add(&b);
    seats.add(&c);

    a.recordRoundScore(77);
    check(ScoreManager::findGameWinner(GameMode::Score, 100, 1, seats) == nullptr,
          "winner (score): nobody reached the limit yet");

    a.recordRoundScore(30);
    check(ScoreManager::findGameWinner(GameMode::Score, 100, 2, seats) == &a,
          "winner (score): the first player to reach the limit wins");
    check(ScoreManager::findGameWinner(GameMode::Score, 107, 2, seats) == &a,
          "winner (score): reaching the limit exactly is enough");
    check(ScoreManager::findGameWinner(GameMode::Score, 108, 2, seats) == nullptr,
          "winner (score): one point short is not enough");

    check(ScoreManager::findGameWinner(GameMode::Rounds, 3, 2, seats) == nullptr,
          "winner (rounds): the game goes on before the round limit");
    check(ScoreManager::findGameWinner(GameMode::Rounds, 3, 3, seats) == &a,
          "winner (rounds): the highest total wins at the limit");

    HumanPlayer d("D");
    HumanPlayer e("E");
    HumanPlayer f("F");
    DynamicArray<Player*> tied;
    tied.add(&d);
    tied.add(&e);
    tied.add(&f);

    d.recordRoundScore(50);
    e.recordRoundScore(50);
    check(ScoreManager::findGameWinner(GameMode::Rounds, 2, 2, tied) == nullptr,
          "winner (rounds): a tie at the top means an extra round");

    f.recordRoundScore(10);
    check(ScoreManager::findGameWinner(GameMode::Rounds, 2, 3, tied) == nullptr,
          "winner (rounds): still a tie when a lower player scores");

    d.recordRoundScore(5);
    check(ScoreManager::findGameWinner(GameMode::Rounds, 2, 4, tied) == &d,
          "winner (rounds): the tie is broken by an extra round");

    DynamicArray<Player*> nobody;
    check(ScoreManager::findGameWinner(GameMode::Score, 10, 1, nobody) == nullptr,
          "winner: no players gives no winner");
}

static void testRecordGameResults() {
    Account accountA("a", "1111", "A");
    Account accountB("b", "2222", "B");
    HumanPlayer a("A", &accountA);
    HumanPlayer b("B", &accountB);
    BotPlayer bot("Bot_1");
    DynamicArray<Player*> seats;
    seats.add(&a);
    seats.add(&b);
    seats.add(&bot);

    a.recordRoundScore(120);
    b.recordRoundScore(40);
    bot.recordRoundScore(10);

    ScoreManager::recordGameResults(seats, &a);

    check(accountA.getTotalScore() == 120 && accountA.getWins() == 1 &&
          accountA.getGamesPlayed() == 1,
          "results: the winner's account gets score, a win and a game");
    check(accountB.getTotalScore() == 40 && accountB.getWins() == 0 &&
          accountB.getGamesPlayed() == 1,
          "results: the loser's account gets score and a game only");

    ScoreManager::recordGameResults(seats, &bot);
    check(accountA.getGamesPlayed() == 2 && accountA.getWins() == 1 &&
          accountB.getWins() == 0,
          "results: bots have no account and a bot win changes no wins");
}

static void testSetupValidity() {
    DynamicArray<Player*> one;
    makeBots(one, 1);
    Game oneGame(one, GameMode::Score, 100);
    check(!oneGame.isValidSetup() && !oneGame.start() &&
          oneGame.getRound() == nullptr,
          "setup: 1 player is invalid");

    DynamicArray<Player*> two;
    makeBots(two, 2);
    Game twoGame(two, GameMode::Score, 100);
    check(twoGame.isValidSetup() && twoGame.start(), "setup: 2 players are valid");

    DynamicArray<Player*> ten;
    makeBots(ten, 10);
    Game tenGame(ten, GameMode::Rounds, 3);
    check(tenGame.isValidSetup() && tenGame.start(),
          "setup: 10 players are valid");

    DynamicArray<Player*> eleven;
    makeBots(eleven, 11);
    Game elevenGame(eleven, GameMode::Score, 100);
    check(!elevenGame.isValidSetup() && !elevenGame.start(),
          "setup: 11 players are invalid");

    DynamicArray<Player*> three;
    makeBots(three, 3);
    Game zeroLimit(three, GameMode::Score, 0);
    check(!zeroLimit.isValidSetup() && !zeroLimit.start(),
          "setup: a limit of 0 is invalid");

    freeBots(one);
    freeBots(two);
    freeBots(ten);
    freeBots(eleven);
    freeBots(three);
}

static void testGameFlow() {
    DynamicArray<Player*> seats;
    makeBots(seats, 3);

    Game game(seats, GameMode::Score, 100000);
    bool conserved = true;

    check(game.start() && game.getRound() != nullptr && !game.isFinished() &&
          game.getRoundsPlayed() == 0,
          "flow: start creates the first round");
    check(!game.finishRound() && !game.startNextRound(),
          "flow: nothing can be scored or restarted before the round is over");

    check(runRound(game, conserved), "flow: bots finish the first round");

    Player* winner = game.getRound()->getWinner();
    int expected = 0;

    for (int i = 0; i < game.getPlayerCount(); ++i) {
        Player* player = game.getPlayerAt(i);

        if (player != winner) {
            expected += player->handScore();
        }
    }

    check(winner != nullptr && winner->getHandSize() == 0,
          "flow: the winner has no cards left");
    check(game.finishRound() && game.getRoundsPlayed() == 1,
          "flow: finishRound scores the round once");
    check(game.getLastRoundPoints() == expected &&
          winner->getCurrentGameScore() == expected,
          "flow: the winner gets the opponents' hand points");
    check(!game.finishRound() && game.getRoundsPlayed() == 1,
          "flow: finishRound cannot score the same round twice");
    check(scoresConsistent(game), "flow: round scores add up to the totals");
    check(!game.isFinished(), "flow: a high limit keeps the game going");

    check(game.startNextRound() && !game.getRound()->isOver() &&
          game.getRoundsPlayed() == 1,
          "flow: startNextRound begins a new round");
    check(winner->getRoundScores().size() == 1,
          "flow: scores are kept between rounds");
    check(conserved, "flow: all 108 cards stayed accounted for");

    freeBots(seats);
}

static void testBotGames(int playerCount, GameMode mode, int limit, int games,
                         const char* name) {
    DynamicArray<Player*> seats;
    makeBots(seats, playerCount);

    bool allFinished = true;
    bool conserved = true;
    bool consistent = true;
    bool winnerOk = true;

    for (int i = 0; i < games; ++i) {
        Game game(seats, mode, limit);

        if (!playFullGame(game, conserved)) {
            allFinished = false;
            continue;
        }

        if (!scoresConsistent(game)) {
            consistent = false;
        }

        if (!winnerHasTopScore(game)) {
            winnerOk = false;
        }

        Player* winner = game.getGameWinner();

        if (mode == GameMode::Score &&
            winner->getCurrentGameScore() < limit) {
            winnerOk = false;
        }

        if (mode == GameMode::Rounds && game.getRoundsPlayed() < limit) {
            winnerOk = false;
        }
    }

    string label = string(name);

    check(allFinished, (label + ": every game ends").c_str());
    check(conserved, (label + ": 108 cards after every move").c_str());
    check(consistent, (label + ": round scores add up to the totals").c_str());
    check(winnerOk, (label + ": the winner is correct").c_str());

    freeBots(seats);
}

int main() {
    cout << "--- Round scoring ---\n";
    testRoundScoring();

    cout << "\n--- Game winner rules ---\n";
    testGameWinnerRules();

    cout << "\n--- Recording game results ---\n";
    testRecordGameResults();

    cout << "\n--- Game setup ---\n";
    testSetupValidity();

    cout << "\n--- Game flow ---\n";
    testGameFlow();

    cout << "\n--- Full bot games ---\n";
    testBotGames(3, GameMode::Score, 100, 5, "3 bots, 100 points");
    testBotGames(2, GameMode::Score, 50, 5, "2 bots, 50 points");
    testBotGames(10, GameMode::Score, 100, 3, "10 bots, 100 points");
    testBotGames(4, GameMode::Rounds, 3, 5, "4 bots, 3 rounds");

    cout << "\nFailed checks: " << failedChecks << '\n';
    return failedChecks == 0 ? 0 : 1;
}