MSLS_RUNS = 20

ILS_RUNS = 20
ILS_TIME_MS = 1500

def generate():
    yield ("Multiple starts Local Search", [[[], ("RandomSolver", {"seed": seed}), ("MSLS", {"iterations": 400, "localSearch": "ListOfMovesSolver"})] for seed in range(MSLS_RUNS)])
    for perturbation, strength in [
        ("randomMoves", 16),
        ("segment", 8),
        ("doubleBridge", 1),
        ("guided", 24)
    ]:
        yield (f"Iterative Local Search, {perturbation} perturbation with strength {strength}", [[[], ("RandomSolver", {"seed": seed}), ("ILS", {"perturbation": perturbation, "strength": strength, "timeMs": ILS_TIME_MS})] for seed in range(ILS_RUNS)])

