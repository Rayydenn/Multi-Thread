#include "applicgaragewindow.h"
#include "ui_applicgaragewindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <sstream>
#include <iostream>
#include <string>
#include "unistd.h"
#include <algorithm>

// ********************** //
#include "Model.h"
#include "Garage.h"
#include "Client.h"
#include "Car.h"
#include "Employee.h"
#include "Option.h"
#include "Person.h"
#include "Actor.h"

#include "OptionException.h"
#include "PasswordException.h"
// ********************** //

using namespace std;

#define IMAGES_DIR "../images/"
#define ModCSV "../CSVs/models.csv"
#define OptCSV "../CSVs/options.csv"

ApplicGarageWindow::ApplicGarageWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::ApplicGarageWindow)
{
    ui->setupUi(this);
    ::close(2);

    // Configuration de la table des options (projet en cours)
    ui->tableWidgetOptions->setColumnCount(3);
    ui->tableWidgetOptions->setRowCount(5);
    for (int i=0 ; i<5 ; i++) ui->tableWidgetOptions->setRowHeight(i,10);
    QStringList labelsTableOptions;
    labelsTableOptions << "Code" << "Prix" << "Intitulé";
    ui->tableWidgetOptions->setHorizontalHeaderLabels(labelsTableOptions);
    ui->tableWidgetOptions->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetOptions->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetOptions->horizontalHeader()->setVisible(true);
    ui->tableWidgetOptions->horizontalHeader()->setDefaultSectionSize(60);
    ui->tableWidgetOptions->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetOptions->verticalHeader()->setVisible(false);
    ui->tableWidgetOptions->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Configuration de la table des employes (Personnel Garage)
    ui->tableWidgetEmployees->setColumnCount(4);
    ui->tableWidgetEmployees->setRowCount(0);
    QStringList labelsTableEmployes;
    labelsTableEmployes << "Numéro" << "Nom" << "Prénom" << "Fonction";
    ui->tableWidgetEmployees->setHorizontalHeaderLabels(labelsTableEmployes);
    ui->tableWidgetEmployees->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetEmployees->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetEmployees->horizontalHeader()->setVisible(true);
    ui->tableWidgetEmployees->horizontalHeader()->setDefaultSectionSize(80);
    ui->tableWidgetEmployees->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetEmployees->verticalHeader()->setVisible(false);
    ui->tableWidgetEmployees->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Configuration de la table des clients
    ui->tableWidgetClients->setColumnCount(4);
    ui->tableWidgetClients->setRowCount(0);
    QStringList labelsTableClients;
    labelsTableClients << "Numéro" << "Nom" << "Prénom" << "GSM";
    ui->tableWidgetClients->setHorizontalHeaderLabels(labelsTableClients);
    ui->tableWidgetClients->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetClients->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetClients->horizontalHeader()->setVisible(true);
    ui->tableWidgetClients->horizontalHeader()->setDefaultSectionSize(80);
    ui->tableWidgetClients->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetClients->verticalHeader()->setVisible(false);
    ui->tableWidgetClients->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Configuration de la table des contrats
    ui->tableWidgetContracts->setColumnCount(4);
    ui->tableWidgetContracts->setRowCount(0);
    QStringList labelsTableContrats;
    labelsTableContrats << "Numéro" << "Vendeur" << "Client" << "Voiture";
    ui->tableWidgetContracts->setHorizontalHeaderLabels(labelsTableContrats);
    ui->tableWidgetContracts->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetContracts->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetContracts->horizontalHeader()->setVisible(true);
    ui->tableWidgetContracts->horizontalHeader()->setDefaultSectionSize(80);
    ui->tableWidgetContracts->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetContracts->verticalHeader()->setVisible(false);
    ui->tableWidgetContracts->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Importation des modeles (étape 10)
    auto& g = Garage::getInstance();

    g.importModelsFromCsv(ModCSV);
    g.importOptionsFromCsv(OptCSV);

    int i = 0;
    do
    {
        Model m = g.getModel(i);
        i++;
        if (m.getImage().empty())
            break;
        addAvailableModel(m.getName(), m.getBasePrice());
    } while(true);

    // Importation des options (étape 10)

    clearTableOption();
    i = 0;
    do
    {
        Option o = g.getOption(i);
        i++;
        if (o.getCode().empty())
            break;
        addAvailableOption(o.getLabel(), o.getPrice());
    } while(true);

    // Lecture de config.dat, des employees, clients et contrats (étape 12)
    
    auto& grg = Garage::getInstance();
    grg.load();

    int nbE = grg.getNbEmployees();
    for (int i = 0; i < nbE; ++i)
    {
        Employee e = grg.findEmployeeByIndex(i);
        addTupleTableEmployees(e.tuple());
    }

    int nbC = grg.getNbClients();
    for (int i= 0; i < nbC; ++i)
    {
        Client c = grg.findClientByIndex(i);
        addTupleTableClients(c.tuple());
    }


    int nb = g.getNbContracts();
    for (int i = 0; i < nb; ++i)
    {
        Contract c = grg.findContractByIndex(i);
    
        Employee s = grg.findEmployeeById(c.getIdSeller());
        Client   cl = grg.findClientById(c.getIdClient());

        string sellerName = s.getLastName() + " " + s.getFirstName();
        string clientName = cl.getLastName() + " " + cl.getFirstName();
        addTupleTableContracts(c.tuple(sellerName, clientName));
    }

    setRole(0);
}

ApplicGarageWindow::~ApplicGarageWindow()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Méthodes de la fenêtre /////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::setTitle(string title)
{
    this->setWindowTitle(title.c_str());
}

void ApplicGarageWindow::setRole(int val)
{
    // val = 0 --> personne n'est loggé
    // val = 1 --> un administratif est loggé
    // val = 2 --> un vendeur loggé
    // val = 3 --> on a accès à tout

    if (val == 0)
    {
        ui->actionLogin->setEnabled(true);
        ui->actionLogout->setEnabled(false);
        ui->actionResetPassword->setEnabled(false);

        ui->menuEmployees->setEnabled(false);

        ui->menuClients->setEnabled(false);

        ui->menuCar->setEnabled(false);
        ui->comboBoxAvailableModels->setEnabled(false);
        ui->comboBoxAvailableOptions->setEnabled(false);
        ui->pushButtonOpenProject->setEnabled(false);
        ui->pushButtonNewProject->setEnabled(false);
        ui->pushButtonReduction->setEnabled(false);
        ui->pushButtonSaveProject->setEnabled(false);
        ui->pushButtonAddOption->setEnabled(false);
        ui->pushButtonRemoveOption->setEnabled(false);
        ui->pushButtonSelectModel->setEnabled(false);

        ui->pushButtonShowCar->setEnabled(false);
        ui->pushButtonNewContract->setEnabled(false);
        ui->pushButtonDeleteContract->setEnabled(false);
    }

    if (val == 1)
    {
        ui->actionLogin->setEnabled(false);
        ui->actionLogout->setEnabled(true);
        ui->actionResetPassword->setEnabled(true);

        ui->menuEmployees->setEnabled(true);

        ui->menuClients->setEnabled(false);

        ui->menuCar->setEnabled(false);
        ui->comboBoxAvailableModels->setEnabled(false);
        ui->comboBoxAvailableOptions->setEnabled(false);
        ui->pushButtonOpenProject->setEnabled(false);
        ui->pushButtonNewProject->setEnabled(false);
        ui->pushButtonReduction->setEnabled(false);
        ui->pushButtonSaveProject->setEnabled(false);
        ui->pushButtonAddOption->setEnabled(false);
        ui->pushButtonRemoveOption->setEnabled(false);
        ui->pushButtonSelectModel->setEnabled(false);

        ui->pushButtonShowCar->setEnabled(true);
        ui->pushButtonNewContract->setEnabled(false);
        ui->pushButtonDeleteContract->setEnabled(true);
    }

    if (val == 2)
    {
        ui->actionLogin->setEnabled(false);
        ui->actionLogout->setEnabled(true);
        ui->actionResetPassword->setEnabled(true);

        ui->menuEmployees->setEnabled(false);

        ui->menuClients->setEnabled(true);

        ui->menuCar->setEnabled(true);
        ui->comboBoxAvailableModels->setEnabled(true);
        ui->comboBoxAvailableOptions->setEnabled(true);
        ui->pushButtonOpenProject->setEnabled(true);
        ui->pushButtonNewProject->setEnabled(true);
        ui->pushButtonReduction->setEnabled(true);
        ui->pushButtonSaveProject->setEnabled(true);
        ui->pushButtonAddOption->setEnabled(true);
        ui->pushButtonRemoveOption->setEnabled(true);
        ui->pushButtonSelectModel->setEnabled(true);

        ui->pushButtonShowCar->setEnabled(true);
        ui->pushButtonNewContract->setEnabled(true);
        ui->pushButtonDeleteContract->setEnabled(true);
    }

    if (val == 3)
    {
        ui->actionLogin->setEnabled(true);
        ui->actionLogout->setEnabled(true);
        ui->actionResetPassword->setEnabled(true);

        ui->menuEmployees->setEnabled(true);

        ui->menuClients->setEnabled(true);

        ui->menuCar->setEnabled(true);
        ui->comboBoxAvailableModels->setEnabled(true);
        ui->comboBoxAvailableOptions->setEnabled(true);
        ui->pushButtonOpenProject->setEnabled(true);
        ui->pushButtonNewProject->setEnabled(true);
        ui->pushButtonReduction->setEnabled(true);
        ui->pushButtonSaveProject->setEnabled(true);
        ui->pushButtonAddOption->setEnabled(true);
        ui->pushButtonRemoveOption->setEnabled(true);
        ui->pushButtonSelectModel->setEnabled(true);

        ui->pushButtonShowCar->setEnabled(true);
        ui->pushButtonNewContract->setEnabled(true);
        ui->pushButtonDeleteContract->setEnabled(true);
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Table des options du projet en cours (ne pas modifier) /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::setTableOption(int index,string code,string label,float price)
{
    // Modifie l'option de la ligne correspondant à l'indice fourni dans la table des options de l'interface graphique
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(code.c_str());
    ui->tableWidgetOptions->setItem(index,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    char tmp[20];
    sprintf(tmp,"%.2f",price);
    if (price < 0.0) item->setText("");
    else item->setText(tmp);
    ui->tableWidgetOptions->setItem(index,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setText(label.c_str());
    ui->tableWidgetOptions->setItem(index,2,item);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::clearTableOption()
{
    for (int i=0 ; i<5 ; i++) setTableOption(i);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ApplicGarageWindow::getIndexOptionSelectionTable() const
{
    // retourne l'indice de l'option selectionnee dans la table des options
    QModelIndexList list = ui->tableWidgetOptions->selectionModel()->selectedRows();
    if (list.size() == 0) return -1;
    QModelIndex index = list.at(0);
    int ind = index.row();
    return ind;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Combobox des modèles disponibles (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::addAvailableModel(string name,float basePrice)
{
    // ajoute un modèle dans le combobox correspondant (juste le nom et le prix)
    stringstream ss;
    char temp[20];
    sprintf(temp,"%.2f",basePrice);
    ss << name.c_str() << " (" << temp << ")";
    ui->comboBoxAvailableModels->addItem(ss.str().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::clearComboBoxAvailableModels()
{
    ui->comboBoxAvailableModels->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ApplicGarageWindow::getIndexModelSelectionCombobox() const
{
    // retourne l'indice du modèle selectionné dans le combobox des modèles
    return ui->comboBoxAvailableModels->currentIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Combox des options disponibles (ne pas modifier) ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::addAvailableOption(string label,float price)
{
    // ajoute une option dans le combobox correspondant
    stringstream ss;
    char temp[20];
    sprintf(temp,"%.2f",price);
    ss << label << " (" << temp << ")";
    ui->comboBoxAvailableOptions->addItem(ss.str().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::clearComboBoxAvailableOptions()
{
    ui->comboBoxAvailableOptions->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ApplicGarageWindow::getIndexOptionSelectionCombobox() const
{
    // retourne l'indice de l'option selectionnée dans le combobox des options
    return ui->comboBoxAvailableOptions->currentIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Gestion I/O Projet en cours (ne pas modifier) //////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::setCurrentProjectName(string name)
{
    // Remplit le champ nom de l'interface graphique avec le nom reçu
    ui->lineEditProjectName->setText(name.c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
string ApplicGarageWindow::getCurrentProjectName() const
{
    return ui->lineEditProjectName->text().toStdString();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::setModel(string name,int power,int engine,float basePrice,string modelImage)
{
    // Remplit les champs correspondants dans l'interface graphique
    // pour le moteur (int) : 0=Essence, 1=Diesel, 2=Electrique, 3=Hybride
    ui->lineEditModelName->setText(name.c_str());
    ui->lineEditPower->setText(to_string(power).c_str());
    char tmp[20];
    sprintf(tmp,"%.2f",basePrice);
    ui->lineEditBasePrice->setText(tmp);

    ui->radioButtonPetrol->setChecked(false);
    ui->radioButtonDiesel->setChecked(false);
    ui->radioButtonElectric->setChecked(false);
    ui->radioButtonHybrid->setChecked(false);

    if (engine == 0) ui->radioButtonPetrol->setChecked(true);
    if (engine == 1) ui->radioButtonDiesel->setChecked(true);
    if (engine == 2) ui->radioButtonElectric->setChecked(true);
    if (engine == 3) ui->radioButtonHybrid->setChecked(true);

    // Met à jour l'image du modèle
    string cheminComplet = IMAGES_DIR + modelImage;
    QLabel* label = new QLabel();
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    label->setScaledContents(true);
    QPixmap *pixmap_img = new QPixmap(cheminComplet.c_str());
    label->setPixmap(*pixmap_img);
    label->resize(label->pixmap()->size());
    ui->scrollArea->setWidget(label);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::setPrice(float price)
{
  char tmp[20];
  sprintf(tmp,"%.2f",price);
  ui->lineEditPrice->setText(tmp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table des Employes (ne pas modifier) //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::addTupleTableEmployees(string tuple)
{
    // Controle du tuple recu et parsing
    char Tuple[200];
    strncpy(Tuple,tuple.c_str(),199);
    char* tmp;

    if ((tmp = strtok(Tuple,";")) == NULL) return;
    char id[20];
    strcpy(id,tmp);

    if ((tmp = strtok(NULL,";")) == NULL) return;
    char name[40];
    strcpy(name,tmp);

    if ((tmp = strtok(NULL,";")) == NULL) return;
    char firstName[40];
    strcpy(firstName,tmp);

    if ((tmp = strtok(NULL,";")) == NULL) return;
    char fonction[40];
    strcpy(fonction,tmp);

    // Ajout possible
    int nbLines = ui->tableWidgetEmployees->rowCount();
    nbLines++;
    ui->tableWidgetEmployees->setRowCount(nbLines);
    ui->tableWidgetEmployees->setRowHeight(nbLines-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(id);
    ui->tableWidgetEmployees->setItem(nbLines-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setText(name);
    ui->tableWidgetEmployees->setItem(nbLines-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setText(firstName);
    ui->tableWidgetEmployees->setItem(nbLines-1,2,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setText(fonction);
    ui->tableWidgetEmployees->setItem(nbLines-1,3,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::clearTableEmployees()
{
    ui->tableWidgetEmployees->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ApplicGarageWindow::getIndexEmployeeSelectionTable()
{
    QModelIndexList liste = ui->tableWidgetEmployees->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int ind = index.row();
    return ind;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table des Clients (ne pas modifier) ///////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::addTupleTableClients(string tuple)
{
    // Controle du tuple recu et parsing
    char Tuple[200];
    strncpy(Tuple,tuple.c_str(),199);
    char* tmp;

    if ((tmp = strtok(Tuple,";")) == NULL) return;
    char id[20];
    strcpy(id,tmp);

    if ((tmp = strtok(NULL,";")) == NULL) return;
    char lastName[40];
    strcpy(lastName,tmp);

    if ((tmp = strtok(NULL,";")) == NULL) return;
    char firstName[40];
    strcpy(firstName,tmp);

    if ((tmp = strtok(NULL,";")) == NULL) return;
    char gsm[40];
    strcpy(gsm,tmp);

    // Ajout possible
    int nbLines = ui->tableWidgetClients->rowCount();
    nbLines++;
    ui->tableWidgetClients->setRowCount(nbLines);
    ui->tableWidgetClients->setRowHeight(nbLines-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(id);
    ui->tableWidgetClients->setItem(nbLines-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setText(lastName);
    ui->tableWidgetClients->setItem(nbLines-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setText(firstName);
    ui->tableWidgetClients->setItem(nbLines-1,2,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setText(gsm);
    ui->tableWidgetClients->setItem(nbLines-1,3,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::clearTableClients()
{
    ui->tableWidgetClients->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ApplicGarageWindow::getIndexClientSelectionTable()
{
    QModelIndexList liste = ui->tableWidgetClients->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int ind = index.row();
    return ind;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table des Contrats (ne pas modifier) //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::addTupleTableContracts(string tuple)
{
    // Controle du tuple recu et parsing
    char Tuple[200];
    strncpy(Tuple,tuple.c_str(),199);
    char* tmp;

    if ((tmp = strtok(Tuple,";")) == NULL) return;
    char id[20];
    strcpy(id,tmp);

    if ((tmp = strtok(NULL,";")) == NULL) return;
    char seller[40];
    strcpy(seller,tmp);

    if ((tmp = strtok(NULL,";")) == NULL) return;
    char client[40];
    strcpy(client,tmp);

    if ((tmp = strtok(NULL,";")) == NULL) return;
    char car[40];
    strcpy(car,tmp);

    // Ajout possible
    int nbLines = ui->tableWidgetContracts->rowCount();
    nbLines++;
    ui->tableWidgetContracts->setRowCount(nbLines);
    ui->tableWidgetContracts->setRowHeight(nbLines-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(id);
    ui->tableWidgetContracts->setItem(nbLines-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setText(seller);
    ui->tableWidgetContracts->setItem(nbLines-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setText(client);
    ui->tableWidgetContracts->setItem(nbLines-1,2,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setText(car);
    ui->tableWidgetContracts->setItem(nbLines-1,3,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::clearTableContracts()
{
    ui->tableWidgetContracts->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ApplicGarageWindow::getIndexContractSelectionTable()
{
    QModelIndexList liste = ui->tableWidgetContracts->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int ind = index.row();
    return ind;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::dialogMessage(const char* title,const char* message)
{
   QMessageBox::information(this,title,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::dialogError(const char* title,const char* message)
{
   QMessageBox::critical(this,title,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
string ApplicGarageWindow::dialogPromptText(const char* title,const char* question)
{
    QString chaine = QInputDialog::getText(this,title,question);
    return chaine.toStdString();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ApplicGarageWindow::dialogPromptInt(const char* title,const char* question)
{
    return QInputDialog::getInt(this,title,question);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float ApplicGarageWindow::dialogPromptFloat(const char* title,const char* question)
{
    return QInputDialog::getDouble(this,title,question);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les items de menu //////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionQuit_triggered()
{
    // TO DO (étape 12)

    auto& g = Garage::getInstance();
    g.save();

    QApplication::exit();
    cout << ">>> Clic sur item Quitter <<<" << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::closeEvent(QCloseEvent *event)
{
    // TO DO (étape 12)

    auto& g = Garage::getInstance();
    g.save();

    cout << ">>> Clic sur croix de fenetre <<<" << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionNewModel_triggered()
{
    // TO DO (étape 9)

    string modelName = this->dialogPromptText("Nouveau modèle","Nom :");
    if (modelName.empty())
    {
        dialogError("Model", "Le nom du modèle ne doit pas être vide !");
        return;
    }

    int power = this->dialogPromptInt("Nouveau modèle","Puissance :");
    if (power < 0)
    {
        dialogError("Model", "La puissance du modèle ne doit pas être négative !");
        return;
    }

    int engine = this->dialogPromptInt("Nouveau modèle :","Moteur (0=essence,1=diesel,2=électrique,3=hybride) :");
    if (engine < 0 || engine > 3)
    {
        dialogError("Model", "Le moteur encodé est erroné !");
        return;
    }

    float basePrice = this->dialogPromptFloat("Nouveau modèle","Prix de base :");
    if (basePrice < 0)
    {
        dialogError("Model", "Le prix ne doit pas être négative !");
        return;
    }

    string image = this->dialogPromptText("Nouveau modèle","Nom du fichier de l'image :");

    vector<string> imagesList = {
    "208",
    "2008",
    "3008",
    "308",
    "508",
    "508SW"
    };

    if (std::find(imagesList.begin(), imagesList.end(), image) == imagesList.end())
    {
        dialogError("Model", "L'image ne correspond pas à l'une des images données !");
        return;
    }

    Model m;

    const char* nom;

    nom = modelName.c_str();

    m.setName(nom);
    m.setPower(power);
    m.setEngine(engine);
    m.setBasePrice(basePrice);
    m.setImage(image);


    Garage::getInstance().addModel(m);

    addAvailableModel(modelName, basePrice);

    cout << ">>> Clic sur item NewModel <<<" << endl;
    cout << "modelName = " << modelName << endl;
    cout << "power = " << power << endl;
    cout << "engine = " << engine << endl;
    cout << "basePrice = " << basePrice << endl;
    cout << "image = " << image << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionNewOption_triggered()
{
    // TO DO (étape 9)

    string code = this->dialogPromptText("Nouvelle option","Code :");
    if (code.empty())
    {
        dialogError("Option", "Le code de l'option ne doit pas être vide !");
        return;
    }

    if (code.size() < 4 || code.size() > 4)
    {
        dialogError("Option", "Le code doit être composé de 4 caractères !");
        return;
    }

    string label = this->dialogPromptText("Nouvelle option","Intitule :");
    if (label.empty())
    {
        dialogError("Option", "Le nom de l'option ne doit pas être vide !");
        return;
    }

    float price = this->dialogPromptFloat("Nouvelle option","Prix :");
    if (price < 0)
    {
        dialogError("Option", "Le prix ne doit pas être négatif");
        return;
    }

    Option o;

    o.setCode(code);
    o.setLabel(label);
    o.setPrice(price);

    Garage::getInstance().addOption(o);

    addAvailableOption(label, price);

    cout << ">>> Clic sur item NewOption <<<" << endl;
    cout << "code = " << code << endl;
    cout << "label = " << label << endl;
    cout << "price = " << price << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionAddEmployee_triggered()
{
    // TO DO (étape 11)

    string lastName = this->dialogPromptText("Nouvel employé","Nom :");
    if (lastName.empty())
    {
        dialogError("Employee", "Le nom de l'employee ne doit pas être vide !");
        return;
    }

    string firstName = this->dialogPromptText("Nouvel employé","Prénom :");
    if (firstName.empty())
    {
        dialogError("Employee", "Le prenom de l'employee ne doit pas être vide !");
        return;
    }

    string login = this->dialogPromptText("Nouvel employé","Login :");
    if (login.empty())
    {
        dialogError("Employee", "Le login de l'employee ne doit pas être vide !");
        return;
    }

    int type = this->dialogPromptInt("Nouvel employé","Fonction (0=administratif,1=vendeur) :");
    if (type != 0 && type != 1)
    {
        dialogError("Employee", "Le type de l'employee doit être 0 pour administratif ou 1 pour vendeur !");
        return;
    }

    Employee e;

    e.setLastName(lastName);
    e.setFirstName(firstName);
    e.setLogin(login);
    e.setRole(type);

    Garage::getInstance().addEmployee(lastName, firstName, login, type);

    // mise à jour de la table des employés
    clearTableEmployees();

    Garage &g = Garage::getInstance();
    int nbE = g.getNbEmployees();
    for (int i = 0; i < nbE; ++i)
    {
        Employee e = g.findEmployeeByIndex(i);
        addTupleTableEmployees(e.tuple());
    }


    cout << ">>> Clic sur item AddEmployee <<<" << endl;
    cout << "lastName = " << lastName << endl;
    cout << "firstName = " << firstName << endl;
    cout << "login = " << login << endl;
    cout << "type = " << type << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionDeleteEmployeeById_triggered()
{
    // TO DO (étape 11)
    cout << ">>> Clic sur item DeleteEmployeeById <<<" << endl;

    int id = dialogPromptInt("Supprimer employé", "Id de l'employé à supprimer :");

    Garage::getInstance().deleteEmployeeById(id);

    clearTableEmployees();

    Garage &g = Garage::getInstance();
    int nbE = g.getNbEmployees();
    for (int i = 0; i < nbE; ++i)
    {
        Employee e = g.findEmployeeByIndex(i);
        addTupleTableEmployees(e.tuple());
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionDeleteEmployeeByIndex_triggered()
{
    // TO DO (étape 11)
    cout << ">>> Clic sur item DeleteEmployeeByIndex <<<" << endl;

    int ind = getIndexEmployeeSelectionTable();
    if (ind == -1)
    {
        dialogError("Supprimer employé",
                    "Vous devez sélectionner un employé dans la table !");
        return;
    }
    
    Garage::getInstance().deleteEmployeeByIndex(ind);

    clearTableEmployees();

    Garage &g = Garage::getInstance();
    int nbE = g.getNbEmployees();
    for (int i = 0; i < nbE; ++i)
    {
        Employee e = g.findEmployeeByIndex(i);
        addTupleTableEmployees(e.tuple());
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionAddClient_triggered()
{
    // TO DO (étape 11)
    cout << ">>> Clic sur item AddClient <<<" << endl;

    string lastName = this->dialogPromptText("Nouvel Client","Nom :");
    if (lastName.empty())
    {
        dialogError("Client", "Le nom du client ne doit pas être vide !");
        return;
    }

    string firstName = this->dialogPromptText("Nouvel Client","Prénom :");
    if (firstName.empty())
    {
        dialogError("Client", "Le prenom du client ne doit pas être vide !");
        return;
    }

    string gsm = this->dialogPromptText("Nouvel Client","Numéro de GSM :");
    if (gsm.length() < 6)
    {
        dialogError("Client", "Le numéro de GSM est invalide");
        return;
    }

    Client c;

    c.setLastName(lastName);
    c.setFirstName(firstName);
    c.setGsm(gsm);

    Garage::getInstance().addClient(lastName, firstName, gsm);
    Garage &g = Garage::getInstance();
    clearTableClients();

    int nbc = g.getNbClients();
    for (int i = 0; i < nbc; ++i)
    {
        Client e = g.findClientByIndex(i);
        addTupleTableClients(e.tuple());
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionDeleteClientById_triggered()
{
    // TO DO (étape 11)
    cout << ">>> Clic sur item DeleteClientById <<<" << endl;

    int id = this->dialogPromptInt("Suppression par ID", "ID : ");
    if (id < 0)
    {
        dialogError("Client", "L'id entré est invalide");
        return;
    }


    Client c = Garage::getInstance().findClientById(id);

    if (c.getId() == 0)
    {
        dialogError("Client", "Aucun Client de cet ID");
        return;
    }


    Garage::getInstance().deleteClientById(id);

    clearTableClients();
    Garage &g = Garage::getInstance();

    int nbc = g.getNbClients();
    for (int i = 0; i < nbc; ++i)
    {
        Client e = g.findClientByIndex(i);
        addTupleTableClients(e.tuple());
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionDeleteClientByIndex_triggered()
{
    // TO DO (étape 11)
    cout << ">>> Clic sur item DeleteClientByIndex <<<" << endl;

    int indice = getIndexClientSelectionTable();
    if (indice == -1)
    {
        dialogError("Client", "L'indice selectionné est invalide");
        return;
    }

    Garage::getInstance().deleteClientByIndex(indice);

    clearTableClients();

    Garage &g = Garage::getInstance();
    int nbc = g.getNbClients();
    for (int i = 0; i < nbc; ++i)
    {
        Client e = g.findClientByIndex(i);
        addTupleTableClients(e.tuple());
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionLogin_triggered()
{
    // TO DO (étape 11)
    cout << ">>> Clic sur item Login <<<" << endl;

    string login = dialogPromptText("Login", "Entrez votre login :");
    if (login.empty())
    {
        dialogError("Login", "Le login ne peut pas être vide.");
        return;
    }

    Garage &g = Garage::getInstance();
    int nbE = g.getNbEmployees();

    Employee found;
    bool ok = false;

    for (int i = 0; i < nbE; ++i)
    {
        Employee e = g.findEmployeeByIndex(i);
        if (e.getLogin() == login)
        {
            found = e;
            ok = true;
            break;
        }
    }


    if (!ok)
    {
        dialogError("Login", "Aucun employé avec ce login.");
        return;
    }

    Garage::idLoggedEmployee = found.getId();

    try
    {
        string pwd = found.getPassword();        
        string saisie = dialogPromptText("Login", "Mot de passe :");

        if (saisie != pwd)
        {
            dialogError("Login", "Mot de passe incorrect.");
            Garage::idLoggedEmployee = -1;
            return;
        }
    }

    catch (PasswordException &)
    {
        dialogMessage("Mot de passe","Vous n'avez pas encore de mot de passe, vous devez en définir un.");

        bool okPwd = false;
        Garage &g = Garage::getInstance();

        while (!okPwd)
        {
            string newPwd = dialogPromptText("Mot de passe","Encodez un nouveau mot de passe :");
            try
            {
                Employee e = g.findEmployeeById(found.getId());
                e.setPassword(newPwd);
                
                g.deleteEmployeeById(found.getId());
                g.addEmployee(found.getLastName(), found.getFirstName(), found.getLogin(), found.getRole(), found.getId(), &newPwd);
                okPwd = true;
            }
            
            catch (PasswordException &pe)
            {
                dialogError("Mot de passe ", pe.getmsg().c_str());
            }
        }
    }

    if (found.getRole() == Employee::ADMINISTRATIVE)
    {
        clearTableEmployees();
        clearTableClients();
        clearTableContracts();
        setRole(1);
        auto& grg = Garage::getInstance();

        int nbE = grg.getNbEmployees();
        for (int i = 0; i < nbE; ++i)
        {
            Employee e = grg.findEmployeeByIndex(i);
            addTupleTableEmployees(e.tuple());
        }

        int nbC = grg.getNbClients();
        for (int i= 0; i < nbC; ++i)
        {
            Client c = grg.findClientByIndex(i);
            addTupleTableClients(c.tuple());
        }

    }
    else
    {
        clearTableEmployees();
        clearTableClients();
        clearTableContracts();
        setRole(2);
        auto& grg = Garage::getInstance();

        int nbE = grg.getNbEmployees();
        for (int i = 0; i < nbE; ++i)
        {
            Employee e = grg.findEmployeeByIndex(i);
            addTupleTableEmployees(e.tuple());
        }

        int nbC = grg.getNbClients();
        for (int i= 0; i < nbC; ++i)
        {
            Client c = grg.findClientByIndex(i);
            addTupleTableClients(c.tuple());
        }


        int nb = grg.getNbContracts();
        for (int i = 0; i < nb; ++i)
        {
            Contract c = grg.findContractByIndex(i);
        
            Employee s = grg.findEmployeeById(c.getIdSeller());
            Client   cl = grg.findClientById(c.getIdClient());

            string sellerName = s.getLastName() + " " + s.getFirstName();
            string clientName = cl.getLastName() + " " + cl.getFirstName();
            addTupleTableContracts(c.tuple(sellerName, clientName));
        }

    }

    string titre = found.getLastName() + " " + found.getFirstName();
    setTitle(titre);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionLogout_triggered()
{
    // TO DO (étape 11)
    cout << ">>> Clic sur item Logout <<<" << endl;

    setTitle("Application Garage");

    setRole(0);

    clearTableEmployees();
    clearTableClients();
    clearTableContracts();
    clearTableOption();


    auto& g = Garage::getInstance();

    g.importModelsFromCsv(ModCSV);
    g.importOptionsFromCsv(OptCSV);

    int i = 0;
    do
    {
        Model m = g.getModel(i);
        i++;
        if (m.getImage().empty())
            break;
        addAvailableModel(m.getName(), m.getBasePrice());
    } while(true);

    i = 0;
    do
    {
        Option o = g.getOption(i);
        i++;
        if (o.getCode().empty())
            break;
        addAvailableOption(o.getLabel(), o.getPrice());
    } while(true);

    Garage::idLoggedEmployee = -1;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_actionResetPassword_triggered()
{
    // TO DO (étape 11)
    cout << ">>> clic sur item ResetPassword <<<" << endl;

    int id = Garage::idLoggedEmployee;
    if (id == -1)
    {
        dialogError("Reset mot de passe","Aucun employé n'est actuellement connecté.");
        return;
    }

    Garage &g = Garage::getInstance();
    Employee e = g.findEmployeeById(id);
    if (e.getId() == 0)
    {
        dialogError("Reset mot de passe","Employé connecté introuvable.");
        return;
    }

    e.resetPassword();

    g.deleteEmployeeById(id);
    g.displayEmployees();
    g.updateEmployee(e, id);

    dialogMessage("Reset mot de passe","Votre mot de passe a été réinitialisé.");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_pushButtonSelectModel_clicked()
{
    // TO DO (étape 9)
    cout << ">>> Clic sur bouton SelectModel <<<" << endl;

    int indice;

    indice = getIndexModelSelectionCombobox();
    if (indice == -1)
    {
        dialogError("Model", "Aucun Modele n'est selectionné !");
        return;
    }

    Garage& g = Garage::getInstance();

    Model m = g.getModel(indice);

    Car& c = g.getCurrentProject();

    c.setModel(m);

    setModel(m.getName(), m.getPower(), m.getEngine(), m.getBasePrice(), m.getImage());

    setPrice(c.getPrice());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_pushButtonAddOption_clicked()
{
    // TO DO (étape 9)
    cout << ">>> Clic sur bouton AddOption <<<" << endl;
    int indice;

    indice = getIndexOptionSelectionCombobox();
    if (indice == -1)
    {
        dialogError("Option", "Aucune Option n'est selectionnée !");
        return;
    }
    Garage& g = Garage::getInstance();

    Option o = g.getOption(indice);

    Car& c = g.getCurrentProject();

    int index;
    string verifcode = o.getCode();

    for (int i = 0; i< 5;i++)
    {
        if (c[i] == nullptr)
        {
            index = i;
            break;
        }
        if (c[i]->getCode() == verifcode)
        {
            return;
        }
    }


    if (index == -1)
    {
        dialogError("Erreur Option Add", "Vous ne pouvez pas ajouter plus que 5 options");
        return;
    }


    try
    {
        c.addOption(o);

    }
    catch(const OptionException& op)
    {
        dialogError("Erreur Option", op.getmsg().c_str());
    }

    setTableOption(index, o.getCode(), o.getLabel(), o.getPrice());

    setPrice(c.getPrice());        

    cout << "Option " << o.getCode() << " ajoutée" << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_pushButtonRemoveOption_clicked()
{
    // TO DO (étape 9)
    cout << ">>> Clic sur bouton RemoveOption <<<" << endl;
    int indice;

    indice = getIndexOptionSelectionTable();
    if (indice == -1)
    {
        dialogError("Option", "Aucune Option n'est selectionnée !");
        return;
    }
    Garage& g = Garage::getInstance();

    Option o = g.getOption(indice);

    Car& c = g.getCurrentProject();


    if (c[indice] != nullptr)
    {
        try
        {
            string code = c[indice]->getCode();
            c.removeOption(code);
        }
        catch(const OptionException& op)
        {
            dialogError("Erreur Option", op.getmsg().c_str());
        }

        setTableOption(indice);

        setPrice(c.getPrice()); 
    }
    else
    {
        dialogError("Erreur Option Remove", "Aucune option n'est selectionnée !");
        return;
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_pushButtonReduction_clicked()
{
    // TO DO (étape 9)
    cout << ">>> Clic sur bouton Reduction <<<" << endl;

    int indice;

    indice = getIndexOptionSelectionTable();
    if (indice == -1)
    {
        dialogError("Option", "Aucune Option n'est selectionnée !");
        return;
    }
    Garage& g = Garage::getInstance();

    Option o;

    Car& c = g.getCurrentProject();

    if (c[indice] != nullptr)
    {
        o.setLabel(c[indice]->getLabel());
        try
        {
            o.setPrice(c[indice]->getPrice());
            o--;
            c[indice]->setPrice(o.getPrice());
        }
        catch(const OptionException& op)
        {
            dialogError("Erreur Réduction", op.getmsg().c_str());
        }

        setTableOption(indice, c[indice]->getCode(), c[indice]->getLabel(), c[indice]->getPrice());

        setPrice(c.getPrice()); 
    }
    else
    {
        dialogError("Erreur Option Remove", "Aucune option n'est selectionnée !");
        return;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_pushButtonSaveProject_clicked()
{
    // TO DO (étape 9)
    cout << ">>> Clic sur bouton SaveProject <<<" << endl;

    string nomProjet = getCurrentProjectName();

    Car& c = Garage::getInstance().getCurrentProject();

    if (nomProjet == "")
    {
        dialogError("Erreur Saisie", "Veuillez entrer un nom au projet avant d'enregistrer");
        return;
    }

    c.display();

    c.setName(nomProjet);

    c.save();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_pushButtonOpenProject_clicked()
{
    // TO DO (étape 9)
    cout << ">>> Clic sur bouton OpenProject <<<" << endl;
    string nomProjet = getCurrentProjectName();

    auto& g = Garage::getInstance();

    Car& c = g.getCurrentProject();
    c.load(nomProjet);

    const Model& m = c.getModel();

    setModel(string(m.getName()), m.getPower(), m.getEngine(), m.getBasePrice(), m.getImage());

    for (int i = 0; i < 5; i++)
    {
        Option* o = c[i];
        if (o)
        {
            setTableOption(i, o->getCode(), o->getLabel(), o->getPrice());
        }
        else
        {
            setTableOption(i);
        }
    }

    setPrice(c.getPrice());
    setCurrentProjectName(nomProjet);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_pushButtonNewProject_clicked()
{
    // TO DO (étape 9)
    Garage::getInstance().resetCurrentProject();

    Car& c = Garage::getInstance().getCurrentProject();

    clearTableOption();
    clearComboBoxAvailableModels();
    clearComboBoxAvailableOptions();
    setPrice(0);
    setModel("---", 0, 0, float(0), "---");
    setCurrentProjectName("---");
    cout << ">>> Clic sur bouton NewProject <<<" << endl;

    auto& g = Garage::getInstance();

    g.importModelsFromCsv(ModCSV);
    g.importOptionsFromCsv(OptCSV);

    int i = 0;
    do
    {
        Model m = g.getModel(i);
        i++;
        if (m.getImage().empty())
            break;
        addAvailableModel(m.getName(), m.getBasePrice());
    } while(true);

    i = 0;
    do
    {
        Option o = g.getOption(i);
        i++;
        if (o.getCode().empty())
            break;
        addAvailableOption(o.getLabel(), o.getPrice());
    } while(true);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_pushButtonNewContract_clicked()
{
    // TO DO (étape 13)
    cout << ">>> Clic sur bouton NewContract <<<" << endl;

    int idSeller = Garage::idLoggedEmployee;
    if (idSeller == -1)
    {
        dialogError("Contrat", "Aucun vendeur n'est connecté.");
        return;
    }

    Garage &g = Garage::getInstance();
    const Employee& eSeller = g.findEmployeeById(idSeller);
    if (eSeller.getId() == 0 || eSeller.getRole() != Employee::SELLER)
    {
        dialogError("Contrat", "Seul un vendeur peut créer un contrat.");
        return;
    }

    int indClient = getIndexClientSelectionTable();
    if (indClient == -1)
    {
        dialogError("Contrat", "Vous devez sélectionner un client.");
        return;
    }
    const Client& cl = g.findClientByIndex(indClient);

    string projectName = getCurrentProjectName();
    if (projectName.empty())
    {
        dialogError("Contrat", "Aucun projet de voiture n'est en cours.");
        return;
    }

    int idContract = g.addContract(eSeller.getId(), cl.getId(), projectName);

    string sellerName = eSeller.getLastName() + " " + eSeller.getFirstName();
    string clientName = cl.getLastName() + " " + cl.getFirstName();

    Contract c(idContract, eSeller.getId(), cl.getId(), projectName);
    addTupleTableContracts(c.tuple(sellerName, clientName));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_pushButtonDeleteContract_clicked()
{
    // TO DO (étape 13)
    cout << ">>> Clic sur bouton DeleteContract <<<" << endl;

    int ind = getIndexContractSelectionTable();
    if (ind == -1)
    {
        dialogError("Contrat", "Vous devez sélectionner un contrat.");
        return;
    }

    Garage &g = Garage::getInstance();
    g.deleteContractByIndex(ind);

    clearTableContracts();

    int nb = g.getNbContracts();
    for (int i = 0; i < nb; ++i)
    {
        const Contract& c = g.findContractByIndex(i);
    
        Employee s = g.findEmployeeById(c.getIdSeller());
        Client   cl = g.findClientById(c.getIdClient());

        string sellerName = s.getLastName() + " " + s.getFirstName();
        string clientName = cl.getLastName() + " " + cl.getFirstName();
        addTupleTableContracts(c.tuple(sellerName, clientName));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplicGarageWindow::on_pushButtonShowCar_clicked()
{
    // TO DO (étape 13)

    int ind = getIndexContractSelectionTable();
    if (ind == -1)
    {
        dialogError("Contrat", "Vous devez sélectionner un contrat.");
        return;
    }

    Garage &g = Garage::getInstance();
    const Contract& c = g.findContractByIndex(ind);

    auto& ca = g.getCurrentProject();

    ca.load(c.getProjectName());

    const Model& m = ca.getModel();

    setModel(string(m.getName()), m.getPower(), m.getEngine(), m.getBasePrice(), m.getImage());

    for (int i = 0; i < 5; i++)
    {
        Option* o = ca[i];
        if (o)  
        {
            setTableOption(i, o->getCode(), o->getLabel(), o->getPrice());
        }
        else
        {
            setTableOption(i);
        }
    }

    setPrice(ca.getPrice());

    setCurrentProjectName(c.getProjectName());
    cout << ">>> Clic sur bouton ShowCar <<<" << endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Mes méthodes à moi /////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*void ApplicGarageWindow::MAJtableOptions()
{
    for (int i=0 ; i<5 ; i++)
    {
        Option *ptr = Garage::getProjetEnCours()[i];
        if (ptr != NULL) setTableOption(i,ptr->getCode(),ptr->getLabel(),ptr->getPrice());
        else setTableOption(i);
    }
}

void ApplicGarageWindow::MAJprojetEnCours()
{
    setCurrentProjectName(Garage::getProjetEnCours().getName());
    setModel(Garage::getProjetEnCours().getModele().getName(),
              Garage::getProjetEnCours().getModele().getPower(),
              Garage::getProjetEnCours().getModele().getEngine(),
              Garage::getProjetEnCours().getModele().getBasePrice(),
              Garage::getProjetEnCours().getModele().getImage());
    MAJtableOptions();
    setPrice(Garage::getProjetEnCours().getPrice());
}

void ApplicGarageWindow::MAJtableEmployes()
{
    videTableEmployes();
    VecteurTrie<Employe> employes = Garage::getInstance().getEmployes();
    Iterateur<Employe> it(employes);
    while (!it.end())
    {
        addTupleTableEmployees(((Employe)it).Tuple());
        it++;
    }
}

void ApplicGarageWindow::MAJtableClients()
{
    videTableClients();
    VecteurTrie<Client> clients = Garage::getInstance().getClients();
    Iterateur<Client> it(clients);
    while (!it.end())
    {
        ajouteTupleTableClients(((Client)it).Tuple());
        it++;
    }
}

void ApplicGarageWindow::MAJtableContrats()
{
    videTableContrats();
    Vecteur<Contrat> contrats = Garage::getInstance().getContrats();
    Iterateur<Contrat> it(contrats);
    while (!it.end())
    {
        ajouteTupleTableContrats(((Contrat)it).Tuple());
        it++;
    }
}
*/


