import random

# --- CLASSE PAQUET ------------------------------------------------------------

class Paquet:
    def __init__(self):
        self.cartes = []
        self.reinitialiser()

    def generer(self):
        """Crée un nouveau paquet mélangé de 52 cartes (0–51)."""
        self.cartes = list(range(52))
        random.shuffle(self.cartes)

    def reinitialiser(self):
        """Réinitialise et mélange le paquet."""
        self.generer()
        print(f"[SERVEUR] 🆕 Nouveau paquet généré ({self.taille()} cartes)")

    def tirer(self, n=1):
        """Tire `n` cartes. Retourne une liste vide si insuffisantes."""
        if self.taille() < n:
            print(f"[SERVEUR] ❌ Pas assez de cartes pour tirer {n}")
            return []
        cartes = self.cartes[-n:]
        self.cartes = self.cartes[:-n]
        return cartes

    def taille(self):
        return len(self.cartes)

    def est_vide(self):
        return self.taille() == 0

# Instance globale du paquet pour le serveur
paquet = Paquet()

# --- LOGIQUE DES CARTES -----------------------------------------------------

def valeur_carte(index):
    rang = index % 13
    if rang == 0:
        return 11
    elif rang >= 10:
        return 10
    else:
        return rang + 1

def formater_nom_carte(index):
    symboles = ["♣", "♥", "♠", "♦"]
    rangs = ["As", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Valet", "Dame", "Roi"]
    symbole = symboles[(index // 13) % 4]
    rang = rangs[index % 13]
    return f"{rang} {symbole}"

def est_blackjack(cartes):
    if len(cartes) != 2:
        return False
    valeurs = sorted([valeur_carte(c) for c in cartes], reverse=True)
    return valeurs == [11, 10]
