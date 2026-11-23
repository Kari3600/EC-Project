NAME = "lab7"

RUNS = 20
TIME_LIMIT = 2500   # average running time of MSLS on the machine used for the experiment (2558 ms A, 2469 ms B)
RANDOM_RUNS = 200
RANDOM_STARTS = 200

# Every run passes its own seed to the randomized method as well, otherwise the rng of
# MSLS, ILS and LNS would be seeded with the default 0 in every run.

def generate():
    yield ("Greedy Cycle with weighted 2regret", [[[start], ("GreedyCycle_2regret", {"alpha": 0.5})] for start in range(RANDOM_STARTS)])
    yield ("Local Search with list of moves", [[[], ("RandomSolver", {"seed": seed}), ("ListOfMovesSolver", {})] for seed in range(RANDOM_RUNS)])
    yield ("Multiple starts Local Search", [[[], ("MSLS", {"seed": seed, "iterations": 400, "localSearch": "ListOfMovesSolver"})] for seed in range(RUNS)])
    yield ("Iterative Local Search", [[[], ("RandomSolver", {"seed": seed}), ("ILS", {"seed": seed, "perturbation": "randomMoves", "strength": 16, "localSearch": "ListOfMovesSolver", "timeMs": TIME_LIMIT})] for seed in range(RUNS)])
    for dname, destroyer in [("random", "random"), ("1 random chain", "randomChain --chainCount 1"), ("5 random chains", "randomChain --chainCount 5")]:
        for ppname, post_processor in [("no", None), ("local search", "ListOfMovesSolver")]:
            yield (f"Large Neighborhood Search, {dname} destroyer, {ppname} postprocessor", [[
                [],
                ("RandomSolver", {"seed": seed}),
                ("ListOfMovesSolver", {}),
                ("LNS", {"seed": seed, "timeMs": TIME_LIMIT, "discardPercent": 0.3, "destroyer": destroyer, "repairer": "GreedyCycle_2regret --alpha 0.5", "postprocessor": post_processor})
            ] for seed in range(RUNS)])

def measurements():
    # the first entry of the trace is the starting solution, every later one is one iteration
    yield ("Iterations of the main loop", lambda d: len(d["trace"]) - 1 if "trace" in d else None, lambda v: f"{v:,}".replace(",","\\,"))
