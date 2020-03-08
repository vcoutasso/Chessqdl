#include "engine.hpp"
#include "utils.hpp"

#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

using namespace chessqdl;


/**
 * @details Starts a new standard game of chess with the engine as \p color pieces
 */
Engine::Engine(enumColor color, int depth, bool v, bool p) {
	bitboard = Bitboard();
	pieceColor = color;
	depthLevel = depth;
	beVerbose = v;
	pvp = p;
}


/**
 * @details Sets up a game of chess according to the \p fen argument with the engine as \p color pieces
 * @fixme not all elements of the fen string are handled
 */
Engine::Engine(std::string fen, enumColor color, int depth, bool v, bool p) {
	bitboard = Bitboard(fen);
	pieceColor = color;
	toMove = (fen.substr(fen.find(' ') + 1, 1) == "w") ? nWhite : nBlack;
	depthLevel = depth;
	beVerbose = v;
	pvp = p;
}


/**
 * @details Prints the current start of the board to stdout
 */
void Engine::printBoard() {
	bitboard.printBoard();
}


/**
 * @details Returns Engine::toMove
 */
enumColor Engine::getToMove() {
	return toMove;
}


/**
 * @details Sets the new max traversal depth of the moves tree to \p nana
 */
void Engine::setDepth(int n) {
	if (n > 0) {
		std::cout << "New max search depth: " << n << std::endl;
		depthLevel = n;
	}
}


/**
 * @details Main interface to the engine. Allows the player to interact with the engine with the options: <br>
 * <b> print </b> calls Engine::printBoard() and prints the current state of the board to stdout using unicode symbols <br>
 * <b> move </b> or <b> mv </b> expects a string after the keyword with the move to be made. The move will only be made if a) it's your turn to move the desired pieces and b) the move is valid <br>
 * <b> undo </b> takes back the latest move made. Can take an argument after the keyword to specify the amount of moves to be unmade <br>
 * <b> depth </b> or <b> set_depth </b> specifies the new maximum search depth of the algorithm. The higher the maximum depth, the higher the difficulty of the engine <br>
 * <b> exit </b> or <b> quit </b> exits the game without saving the progress <br>
 */
void Engine::parser() {
	std::string input;

	while(true) {
		if (pieceColor == toMove && !pvp) {
			if (this->beVerbose) std::cout << std::endl << "Searching for the next move..." << std::endl;
			makeMove(getBestMove(depthLevel, pieceColor));
			printBoard();
		}

		std::cout << "> ";
		std::cin >> input;

		if (input == "print" || input == "print_board")
			printBoard();
		else if (input == "move" || input == "mv") {
			// Reads the movement to be made
			std::cin >> input;
			makeMove(input);
			printBoard();
		} else if (input == "undo") {
			int num = 1;
			readInteger(num);
			for (int i = 0; i < num; i++) {
				if (!moveHistory.empty())
					takeMove();
				else {
					std::cout << "Move history is empty!" << std::endl;
					break;
				}
			}
		} else if (input == "restart") {
			while (!moveHistory.empty())
				takeMove();
		} else if (input == "depth" || input == "set_depth") {
			int d = 3;
			readInteger(d);
			setDepth(d);
		} else if (input == "exit" || input == "quit")
			break;
		else if (input == "list") {
			auto moves = MoveGenerator::getPseudoLegalMoves(bitboard.getBitBoards(), toMove);
			for (auto &mv : moves)
				std::cout << mv << std::endl;
		} else if (input == "hint") {
			std::cout << getBestMove(depthLevel, toMove) << std::endl;
		} else if (input == "help") {
			std::cout << "print_board (print for short) - prints out the current state of the board" << std::endl;
			std::cout << "move (mv for short)           - makes a movement if valid. 'move' and 'mv' can be omitted" << std::endl;
			std::cout << "set_depth (depth for short)   - specifies the search depth of the minimax algorithm. Used to adjust difficulty of the engine" << std::endl;
			std::cout << "list                          - prints out a list of valid moves in the expected format" << std::endl;
			std::cout << "hint                          - prints the move that the engine would make" << std::endl;
			std::cout << "undo                          - takes a movement from the stack. Accepts an integer as argument to specify the amount of moves to be taken" << std::endl;
			std::cout << "restart                       - starts a new match with the standard board configuration" << std::endl;
			std::cout << "help                          - prints out this message with information about valid commands" << std::endl;
			std::cout << "exit (or quit)                - exits the game" << std::endl;
		} else {
			auto moves = MoveGenerator::getPseudoLegalMoves(bitboard.getBitBoards(), toMove);
			if (std::find(moves.begin(), moves.end(), input) != moves.end()) {
				makeMove(input);
				printBoard();
			} else {
				std::cout << "'" << input << "' is not a valid command." << std::endl;
				// Flushing stdin
				int ch;
				while ((ch = std::cin.get()) != '\n' && ch != EOF);
			}
		}

		if (bitboard.getKing(nWhite) == 0) {
			std::cout << std::endl << "Game over! Black wins" << std::endl;
			break;
		} else if (bitboard.getKing(nBlack) == 0) {
			std::cout << std::endl << "Game over! White wins" << std::endl;
			break;
		}

	}

}


/**
 * @brief Moves a piece from a square to another and updates the bitboards, the move history and the capture history when necessary
 * @param mv  the move to be made
 * @param verbose  whether or not to print the move made (in algebraic notation) to stdout. This flag is used so that the engine won't flood stdout with all the moves it has made while searching for
 * the optimal one
 */
void Engine::makeMove(std::string mv, bool verbose) {
	auto pseudoLegal = MoveGenerator::getPseudoLegalMoves(bitboard.getBitBoards(), toMove);
	bool found = std::find(pseudoLegal.begin(), pseudoLegal.end(), mv) != pseudoLegal.end();

	enumColor otherPlayer = (toMove == nWhite) ? nBlack : nWhite;

	if (!found)
		std::cout << "Invalid move!" << std::endl;
	else {
		// Source square
		std::string from = mv.substr(0, 2);
		// Destination square
		std::string to = mv.substr(2, 2);

		// Iterators to the respective square name
		auto fromIt = std::find(mapPositions.begin(), mapPositions.end(), from);
		auto toIt = std::find(mapPositions.begin(), mapPositions.end(), to);

		// Index of the respective square
		int fromIdx = std::distance(mapPositions.begin(), fromIt);
		int toIdx = std::distance(mapPositions.begin(), toIt);

		// Type of the piece that is being moved
		enumPiece pieceType = nPawn;

		// It is assumed that the moving piece is a pawn. This loop checks verifies if it's true and if it isn't, it changes the piece type
		for (int i = nPawn; i <= nKing; i++) {
			U64 aux = bitboard.getPiecesAt(i) & bitboard.getPieces(toMove);
			if (aux.test(fromIdx)) {
				pieceType = enumPiece(i);
				break;
			}
		}

		// Inserts the appropriate character into the algebraic notation string
		switch (pieceType) {
			case nKnight:
				mv.insert(0, "N");
				break;
			case nBishop:
				mv.insert(0, "B");
				break;
			case nRook:
				mv.insert(0, "R");
				break;
			case nQueen:
				mv.insert(0, "Q");
				break;
			case nKing:
				mv.insert(0, "K");
				break;
			default:
				break;
		}

		// If a piece is being captured
		if (bitboard.testBit(otherPlayer, toIdx)) {
			for (int i = nPawn; i <= nKing; i++) {
				U64 aux = bitboard.getPiecesAt(i) & bitboard.getPieces(otherPlayer);
				if (aux.test(toIdx)) {
					bitboard.resetBit(i, toIdx);
					captureHistory.push(enumColor(i));
					mv.insert(mv.find_first_of("12345678") + 1, "x");
					break;
				}
			}
			// Removes piece from the board
			bitboard.resetBit(otherPlayer, toIdx);
		}

		// Updates bitboards
		bitboard.resetBit(pieceType, fromIdx);
		bitboard.resetBit(toMove, fromIdx);
		bitboard.setBit(pieceType, toIdx);
		bitboard.setBit(toMove, toIdx);

		// If is promotion
		if (isalpha(mv.back()) && pieceType == nPawn) {
			// Promote to queen by default
			enumPiece promoteTo = nQueen;

			if (mv.back() == 'n') promoteTo = nKnight;
			else if (mv.back() == 'b') promoteTo = nBishop;
			else if (mv.back() == 'r') promoteTo = nRook;

			bitboard.setBit(promoteTo, toIdx);
			bitboard.resetBit(pieceType, toIdx);
		}


		// Updates the bitboard that contains info about both players
		bitboard.updateBitboard();

		// Move number after the string when appropriate (if ply is even)
		if (ply % 2 == 0)
			mv = std::to_string(ply / 2 + 1) + ". " + mv;

		++ply;

		// Updates move history
		moveHistory.push(mv);

		toMove = otherPlayer;

		if (verbose)
			std::cout << mv << std::endl;
	}
}


/**
 * @details Removes the latest entry to the move history and updates the bitboards accordingly.
 */
void Engine::takeMove() {

	if (!moveHistory.empty()) {

		std::string lastMove = moveHistory.top();
		moveHistory.pop();

		if (lastMove.find(" ") != std::string::npos) {
			lastMove = lastMove.substr(lastMove.find(" ") + 1);
		}

		enumPiece pieceType = nPawn;
		bool captured = false;

		// Figures out the piece that was moved
		if (lastMove.find_first_of("NBRQK") != std::string::npos) {
			if (lastMove.at(0) == 'N')
				pieceType = nKnight;
			else if (lastMove.at(0) == 'B')
				pieceType = nBishop;
			else if (lastMove.at(0) == 'R')
				pieceType = nRook;
			else if (lastMove.at(0) == 'Q')
				pieceType = nQueen;
			else if (lastMove.at(0) == 'K')
				pieceType = nKing;

			lastMove.erase(0, 1);
		}

		// If a piece was captured, it has to be placed back in the board
		if (lastMove.find("x") != std::string::npos) {
			captured = true;
			lastMove.erase(lastMove.find("x"), 1);
		}

		// Strings with the name of the squares used (source and destination)
		std::string from = lastMove.substr(0, 2);
		std::string to = lastMove.substr(2, 2);

		// Iterators of the respective square name
		auto fromIt = std::find(mapPositions.begin(), mapPositions.end(), from);
		auto toIt = std::find(mapPositions.begin(), mapPositions.end(), to);

		// Indexes of the respective square name
		int fromIdx = std::distance(mapPositions.begin(), fromIt);
		int toIdx = std::distance(mapPositions.begin(), toIt);

		enumColor hasMoved = toMove == nWhite ? nBlack : nWhite;

		// Updates bitboards
		bitboard.resetBit(pieceType, toIdx);
		bitboard.resetBit(hasMoved, toIdx);
		bitboard.setBit(pieceType, fromIdx);
		bitboard.setBit(hasMoved, fromIdx);

		// If is promotion
		if (isalpha(lastMove.back()) && pieceType == nPawn) {
			// Promote to queen by default
			enumPiece promoteTo = nQueen;

			if (lastMove.back() == 'n') promoteTo = nKnight;
			else if (lastMove.back() == 'b') promoteTo = nBishop;
			else if (lastMove.back() == 'r') promoteTo = nRook;

			bitboard.resetBit(promoteTo, toIdx);
		}

		// "Decaptures" a piece
		if (captured) {
			enumColor capturedPiece = captureHistory.top();
			captureHistory.pop();

			bitboard.setBit(toMove, toIdx);
			bitboard.setBit(capturedPiece, toIdx);
		}

		bitboard.updateBitboard();

		toMove = hasMoved;

		--ply;
	}
}


/**
 * @details Performs a recursive search on the moves tree using the minimax algorithm with alpha-beta pruning and returns the best move it has found
 */
std::string Engine::getBestMove(int depth, enumColor color) {
	std::string bestMove;
	int nodesVisited = 0;

	auto begin = std::chrono::steady_clock::now();

	alphaBetaMax(intMin, intMax, depth, depth, color, nodesVisited, bestMove);

	auto end = std::chrono::steady_clock::now();

	if (this->beVerbose) {
		std::cout << "Best move found: " << bestMove << std::endl;
		std::cout << "Nodes visited: " << nodesVisited << std::endl;
		std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;
	}


	return bestMove;
}


/**
 * @details Minimax implementation
 * @ref https://en.wikipedia.org/wiki/Minimax <br>
 * https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning
 */
int Engine::alphaBetaMax(int alpha, int beta, int depth, int depthLeft, enumColor color, int &nodesVisited, std::string &bestMove) {
	if (depthLeft == 0)
		return evaluateBoard(bitboard.getBitBoards(), color);

	auto allMoves = MoveGenerator::getPseudoLegalMoves(bitboard.getBitBoards(), color);

	auto rng = std::default_random_engine{};
	std::shuffle(std::begin(allMoves), std::end(allMoves), rng);

	enumColor enemyColor = (color == nWhite) ? nBlack : nWhite;

	for (auto &currentMove : allMoves) {

		nodesVisited++;

		makeMove(currentMove, false);
		int score = alphaBetaMin(alpha, beta, depth, depthLeft - 1, enemyColor, nodesVisited, bestMove);
		takeMove();

		if (score >= beta)
			return beta;
		if (score > alpha) {
			alpha = score;
			if (depth == depthLeft)
				bestMove = currentMove;
			//std::cout << "New move found for depth " << depth << " " << currentMove << " score: " << score << std::endl;
		}
	}
	return alpha;
}


/**
 * @details Minimax implementation
 * @ref https://en.wikipedia.org/wiki/Minimax <br>
 * https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning
 */
int Engine::alphaBetaMin(int alpha, int beta, int depth, int depthLeft, enumColor color, int &nodesVisited, std::string &bestMove) {

	if (depthLeft == 0)
		return -evaluateBoard(bitboard.getBitBoards(), color);

	auto allMoves = MoveGenerator::getPseudoLegalMoves(bitboard.getBitBoards(), color);

	auto rng = std::default_random_engine{};
	std::shuffle(std::begin(allMoves), std::end(allMoves), rng);

	enumColor enemyColor = (color == nWhite) ? nBlack : nWhite;

	for (auto &currentMove : allMoves) {

		nodesVisited++;

		makeMove(currentMove, false);
		int score = alphaBetaMax(alpha, beta, depth, depthLeft - 1, enemyColor, nodesVisited, bestMove);
		takeMove();

		if (score <= alpha)
			return alpha;
		if (score < beta) {
			beta = score;
			//std::cout << "New move found for depth " << depth << " " << currentMove << " score: " << score << std::endl;
		}
	}
	return beta;
}
