from utils import valeur_carte, est_blackjack, formater_nom_carte

class Joueur:
    def __init__(self, websocket, nom=""):
        self.websocket = websocket
        self.nom = nom
        self.main = []
        self.est_pret = False
        self.a_termine = False
        self.veut_rejouer = False

    def ajouter_cartes(self, indices):
        for index in indices:
            if not self.a_atteint_limite():
                self.main.append(index)

    def reinitialiser(self):
      self.main = []
      self.score = 0
      self.est_pret = False
      self.a_termine = False
      self.veut_rejouer = False  # On remet à False une fois la nouvelle partie commencée


    def a_atteint_limite(self):
        return len(self.main) >= 12

    def valeur_totale(self):
        total = sum(valeur_carte(c) for c in self.main)
        nb_as = sum(1 for c in self.main if valeur_carte(c) == 11)
        while total > 21 and nb_as > 0:
            total -= 10
            nb_as -= 1
        return total

    def est_blackjack(self):
        return est_blackjack(self.main)

    def get_cartes(self):
        return self.main
    
    def __str__(self):
        cartes_formatees = [formater_nom_carte(i) for i in self.main]
        return " | ".join([
            f"nom = {self.nom}",
            f"main = {cartes_formatees}",
            f"valeur_totale = {self.valeur_totale()}",
            f"blackjack = {self.est_blackjack()}",
            f"est_pret = {self.est_pret}",
            f"a_termine = {self.a_termine}"
        ])
