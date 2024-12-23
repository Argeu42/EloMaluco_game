#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <array>
#include "ObjectRotation.h"
#include "readXML.h"
#include "camera.h"
#include <algorithm>
#include <functional>
#include <vector>
#include <string>
#include <random>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Estrutura para representar um botão
struct Button
{
    std::string text;
    float x, y, width, height;
    void (*callback)();
};

// Estrutura para representar uma face
struct Face {
    std::string colorCode;
    std::string positionCode;
    std::string textureType;
    GLuint textureID;
    bool isVazio;
};

// Vetor para armazenar todas as faces
std::vector<Face> faces;
std::vector<Face> facesOriginals;
std::vector<Button> buttons;

GLfloat vertices[8][6] = {
    {1.0, 0.0, 0.0,  1.0,  0.0,  0.0}, // 0
    {1.0, 1.0, 0.0,  1.0,  0.0,  0.0}, // 1
    {1.0, 1.0, 1.0,  1.0,  0.0,  0.0}, // 2
    {1.0, 0.0, 1.0,  1.0,  0.0,  0.0}, // 3
    {0.0, 0.0, 0.0, -1.0,  0.0,  0.0}, // 4
    {0.0, 1.0, 0.0, -1.0,  0.0,  0.0}, // 5
    {0.0, 1.0, 1.0, -1.0,  0.0,  0.0}, // 6
    {0.0, 0.0, 1.0, -1.0,  0.0,  0.0}  // 7
};

// Mapa de códigos de cores para valores RGB
std::map<std::string, std::array<float, 3>> colorMap = {
    {"vm", {1.0f, 0.0f, 0.0f}}, // Vermelho
    {"vr", {0.0f, 1.0f, 0.0f}}, // Verde
    {"am", {1.0f, 1.0f, 0.0f}}, // Amarelo
    {"br", {1.0f, 1.0f, 1.0f}}, // Branco
    {"vz", {0.0f, 0.0f, 0.0f}}  // Preto
};

// Mapa de códigos de posição para posições
std::map<std::string, std::string> positionMap = {
    {"s", "Superior"},
    {"m1", "Intermediario1"},
    {"m2", "Intermediario2"},
    {"i", "Inferior"},
    {"o", "Vazio"},

};

// Mapeamento dos códigos do XML
std::map<int, std::string> cubeToPositionCode = {
    {0, "s"},
    {1, "m2"},
    {2, "m1"},
    {3, "i"}
};


// Posicionamento dos "observadores"
Camera camera(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.5f, 2.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f));
ObjectRotation objectRotation(glm::vec3(0.5f, 0.0f, 0.5f));
GLfloat light_position[] = { 0.0f, 5.0f, 5.0f, 1.0f };

// Componestes da iluminação
GLfloat mat_ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat high_shininess[] = { 100.0f };

// Declaração das funções

int showMenu();
void display();
void drawUIElements();
void drawButton(const char *text, float x, float y, float width, float height, void (*callback)());
void drawString(const char *str, float x, float y);
void drawFaces();
void initFaces();
void moveFaceVaziaUp();
void moveFaceVaziaDown();
void randomizeFaces();
void updateXMLFromFaces();
GLuint loadTexture(const char *filename);
void initTextures();
void setupLighting();
void setMaterial();
void rotateCube(int cubeIndex, float angle);
bool isGameSolved(const std::vector<Face>& faces);
void keyboard(unsigned char key, int x, int y);
void specialKeyboard(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
bool isGameSolved(const std::vector<Face>& faces);
void restart();

//Variáveis globais utilitárias
bool mouseLeftDown = false;
int mouseX, mouseY;

int windowWidth = 1000;
int windowHeight = 1000;

int selectedCube = -1; // -1 significa nada selecionado
int selectedFace = -1;

bool gameSolved = false;
bool gameInicialized = false;
bool gameChanged = false;
GLuint textureIDs[15]; 

class ActionManager {
private:
    std::vector<std::string> actions;
    size_t currentActionIndex;

public:
    ActionManager() : currentActionIndex(0) {
        actions = readActionsFromXML();
    }

    bool hasNextAction() const {
        return currentActionIndex < actions.size();
    }

    void executeNextAction() {
        if(gameChanged){
            restart();
            return;
        } else if (hasNextAction()) {
            const std::string& action = actions[currentActionIndex++];
            if (action == "rsd") {
                std::cout << "Cubo Superior Rotacionado para a Direita" << std::endl;
                rotateCube(0, -90.0f);
                gameChanged = false;
            } else if (action == "rse") {
                std::cout << "Cubo Superior Rotacionado para a Esquerda" << std::endl;
                rotateCube(0, 90.0f);
                gameChanged = false;
            } else if (action == "rid") {
                std::cout << "Cubo Inferior Rotacionado para a Direita" << std::endl;
                rotateCube(3, -90.0f);
                gameChanged = false;
            } else if (action == "rie") {
                std::cout << "Cubo Inferior Rotacionado para a Esquerda" << std::endl;
                rotateCube(3, 90.0f);
                gameChanged = false;
            } else if (action == "mfc") {
                std::cout << "Face abaixo da vazia movida para Cima" << std::endl;
                moveFaceVaziaDown();
                gameChanged = false;
            } else if (action == "mfb") {
                std::cout << "Face acima da vazia movida para Baixo" << std::endl;
                moveFaceVaziaUp();
                gameChanged = false;
            }
        }
    }

    void resetActionIndex(){
        currentActionIndex = 0;
    }
};

int gameMode;
ActionManager actionManager;
bool podeResolver = 0;

// Função global para o Idle
void idleFunc() {
    if (gameMode == 3) {
        if (actionManager.hasNextAction() && podeResolver) {
            podeResolver = false;
            actionManager.executeNextAction();
            glutPostRedisplay();
        } else if (!actionManager.hasNextAction() && podeResolver == false){
            //gameMode = 2;
        }
    }
    glutPostRedisplay(); // Garante que a tela seja atualizada
}

// Função main
int main(int argc, char **argv)
{
    gameMode = showMenu();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Elo Maluco");

    initTextures(); // Inicializa texturas

    // Inicializa as faces baseado no modo de jogo
    switch (gameMode) {
        case 1:
            randomizeFaces();
            break;
        case 2:
        case 3:
            initFaces();
            break;
    }

    setupLighting(); // Inicializa iluminação

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(motion);

    // Configura a função Idle
    glutIdleFunc(idleFunc);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    glutMainLoop();
    return 0;
};

int showMenu() {
    int choice;
    do {
        std::cout << "\n=== Elo Maluco - Menu ===\n";
        std::cout << "1. Inicie aleatoriamente e jogue manualmente\n";
        std::cout << "2. Iniciar lendo o arquivo XML e jogue manualmente\n";
        std::cout << "3. Inicie lendo o arquivo XML e execute o passo a passo do arquivo XML\n";
        std::cout << "Insira sua escolha (1-3): ";
        std::cin >> choice;
    } while (choice < 1 || choice > 3);
    return choice;
}

// Função display
void display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.1, 100.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glm::mat4 viewMatrix = camera.getViewMatrix();
    glMultMatrixf(glm::value_ptr(viewMatrix));

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glm::mat4 rotationMatrix = objectRotation.getRotationMatrix();
    glMultMatrixf(glm::value_ptr(rotationMatrix));

    drawFaces();

    glDisable(GL_LIGHTING);
    drawUIElements();

    glEnable(GL_LIGHTING);
    
    glutSwapBuffers();
};

// Função para desenhar os elementos da UI do jogo
void drawUIElements() {
    // Salva a matriz de projeção atual
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    // Salva a matriz de modelo atual
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Desenha botões
    buttons.clear();
    if (gameMode == 1 || gameMode == 2){
        drawButton("Move Up", 50, 50, 100, 40, moveFaceVaziaDown);
        drawButton("Move Down", 160, 50, 110, 40, moveFaceVaziaUp);
        drawButton("Rotate left", 280, 50, 110, 40, []() {
            if (selectedCube != -1) {
                rotateCube(selectedCube, 90.0f);
            }
        });
        drawButton("Rotate right", 400, 50, 110, 40, []() {
            if (selectedCube != -1) {
                rotateCube(selectedCube, -90.0f);
            }
        });
        drawButton("Randomize", 520, 50, 110, 40, randomizeFaces);
    }
    if (gameMode == 3){
        drawButton("Avancar", 640, 50, 100, 40, []() {
            if (actionManager.hasNextAction() && podeResolver == false) {
                //actionManager.executeNextAction();
                podeResolver = 1;
                glutPostRedisplay();
            } else {
                std::cout << "Não há mais ações a serem feitas." << std::endl;
            }
        });
    }
    if (gameSolved) {
        drawButton("Voce resolveu o Elo Maluco! Clique aqui para reiniciar!", 280, 910, 480, 40, restart);
    }

    // Restaura as matrizes
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Função para desenhar os botões
void drawButton(const char *text, float x, float y, float width, float height, void (*callback)())
{
    // Salva a matriz de projeção atual
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    // Salva a matriz de modelo atual
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Desenha o texto do botão
    glColor3f(0.0f, 0.0f, 0.0f);
    void *font = GLUT_BITMAP_HELVETICA_18;
    float textWidth = glutBitmapLength(font, (const unsigned char *)text);
    float textHeight = 13;
    float textX = x + (width - textWidth) / 2;
    float textY = y + (height - textHeight) / 2;
    drawString(text, textX, textY);

    // Desenha a borda do botão
    glColor3f(0.4f, 0.4f, 0.4f); // Cinza escuro
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Desenha o background do botão
    glColor3f(0.8f, 0.8f, 0.8f); // Cinza claro
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Restaura as matrizes
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // Guarda as informações do botão
    buttons.push_back({text, x, y, width, height, callback});
}

// Função auxiliar auxiliar para desenhar texto na tela
void drawString(const char *str, float x, float y)
{
    glRasterPos2f(x, y);
    for (const char *c = str; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// Função para desenhar as faces
void drawFaces() {
    for (int i = 0; i < faces.size(); ++i) {
        const Face& face = faces[i];
        if (face.isVazio) continue;

        glPushMatrix();

        int cubeIndex = i / 4;
        if (cubeIndex == 0) glTranslatef(0.0, 3.0, 0.0);
        else if (cubeIndex == 1) glTranslatef(0.0, 2.0, 0.0);
        else if (cubeIndex == 2) glTranslatef(0.0, 1.0, 0.0);
        else if (cubeIndex == 3) glTranslatef(0.0, 0.0, 0.0);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, face.textureID);

        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);  // Cor branca para mostrar a textura corretamente
        int faceIndex = i % 4;
        switch(faceIndex) {
            case 0: // Face frontal
                glNormal3f(1.0f, 0.0f, 0.0f);
                glTexCoord2f(0.0f, 0.0f); glVertex3fv(&vertices[0][0]);
                glTexCoord2f(1.0f, 0.0f); glVertex3fv(&vertices[1][0]);
                glTexCoord2f(1.0f, 1.0f); glVertex3fv(&vertices[2][0]);
                glTexCoord2f(0.0f, 1.0f); glVertex3fv(&vertices[3][0]);
                break;
            case 1: // Face da direita
                glNormal3f(0.0f, 0.0f, 1.0f);
                glTexCoord2f(0.0f, 0.0f); glVertex3fv(&vertices[3][0]);
                glTexCoord2f(1.0f, 0.0f); glVertex3fv(&vertices[2][0]);
                glTexCoord2f(1.0f, 1.0f); glVertex3fv(&vertices[6][0]);
                glTexCoord2f(0.0f, 1.0f); glVertex3fv(&vertices[7][0]);
                break;
            case 2: // Face traseira
                glNormal3f(-1.0f, 0.0f, 0.0f);
                glTexCoord2f(0.0f, 0.0f); glVertex3fv(&vertices[7][0]);
                glTexCoord2f(1.0f, 0.0f); glVertex3fv(&vertices[6][0]);
                glTexCoord2f(1.0f, 1.0f); glVertex3fv(&vertices[5][0]);
                glTexCoord2f(0.0f, 1.0f); glVertex3fv(&vertices[4][0]);
                break;
            case 3: // Face da esquerda
                glNormal3f(0.0f, 0.0f, -1.0f);
                glTexCoord2f(0.0f, 0.0f); glVertex3fv(&vertices[4][0]);
                glTexCoord2f(1.0f, 0.0f); glVertex3fv(&vertices[5][0]);
                glTexCoord2f(1.0f, 1.0f); glVertex3fv(&vertices[1][0]);
                glTexCoord2f(0.0f, 1.0f); glVertex3fv(&vertices[0][0]);
                break;
        }
        glEnd();
        glDisable(GL_TEXTURE_2D);
    
        glBegin(GL_TRIANGLES);
        glColor3f(0.3, 0.3, 0.3); // Cinza
        // Face de baixo
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3fv(&vertices[0][0]);
        glVertex3fv(&vertices[3][0]);
        glVertex3fv(&vertices[7][0]);
        glVertex3fv(&vertices[0][0]);
        glVertex3fv(&vertices[7][0]);
        glVertex3fv(&vertices[4][0]);
        // Face de cima
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3fv(&vertices[1][0]);
        glVertex3fv(&vertices[5][0]);
        glVertex3fv(&vertices[6][0]);
        glVertex3fv(&vertices[1][0]);
        glVertex3fv(&vertices[6][0]);
        glVertex3fv(&vertices[2][0]);
        glEnd();

        // Desenhar a borda se selecionada
        if (selectedCube == cubeIndex) {
            glLineWidth(3.0f);
            glColor3f(0.0f, 0.0f, 1.0f);
            glBegin(GL_LINE_LOOP);
            switch (faceIndex % 4)
            {
                case 0: // Face frontal
                    glVertex3fv(vertices[0]);
                    glVertex3fv(vertices[1]);
                    glVertex3fv(vertices[2]);
                    glVertex3fv(vertices[3]);
                    break;
                case 1: // Face da direita
                    glVertex3fv(vertices[3]);
                    glVertex3fv(vertices[2]);
                    glVertex3fv(vertices[6]);
                    glVertex3fv(vertices[7]);
                    break;
                case 2: // Face traseira 
                    glVertex3fv(vertices[7]);
                    glVertex3fv(vertices[6]);
                    glVertex3fv(vertices[5]);
                    glVertex3fv(vertices[4]);
                    break;
                case 3: // Face da esquerda
                    glVertex3fv(vertices[4]);
                    glVertex3fv(vertices[5]);
                    glVertex3fv(vertices[1]);
                    glVertex3fv(vertices[0]);
                    break;
            }
            glEnd();
            glLineWidth(1.0f);
        }

        glPopMatrix();
    }
}

// Função para inicializar as faces
void initFaces() {
    std::vector<FaceData> faceDatas = readXML();
    faces.clear();
    for (const auto& faceData : faceDatas) {
        Face face;
        face.colorCode = faceData.colorCode;
        face.positionCode = faceData.positionCode;
        face.textureType = faceData.textureType;
        face.isVazio = (face.colorCode == "vz");
        
        if (!face.isVazio && !face.colorCode.empty()){
            std::string colorName;
            if (face.colorCode == "vm") colorName = "red";
            else if (face.colorCode == "am") colorName = "yellow";
            else if (face.colorCode == "vr") colorName = "green";
            else colorName = "white";
            
            std::string textureFile = "./images/circulo_" + colorName + "_" + face.textureType + ".png";
            
            face.textureID = loadTexture(textureFile.c_str());
        } else {
            face.textureID = 0;
        }

        faces.push_back(face);
        if (!gameInicialized) {
            facesOriginals.push_back(face);
        }
    }
    gameInicialized = true;
}

// Move face vazia para cima
void moveFaceVaziaUp() {
    int vaziaIndex = -1;
    for (int i = 0; i < faces.size(); i++) {
        if (faces[i].isVazio) {
            vaziaIndex = i;
            break;
        }
    }

    if (vaziaIndex == -1 || vaziaIndex < 4) return;

    // Trocar as faces
    std::swap(faces[vaziaIndex], faces[vaziaIndex - 4]);

    // Atualizar os códigos de posição
    faces[vaziaIndex].positionCode = cubeToPositionCode[vaziaIndex / 4];
    faces[vaziaIndex - 4].positionCode = "o";

    // Atualizar o XML
    updateXMLFromFaces();
    gameChanged = true; //<- importante para resetar o jogo caso o jogador queira ver a solução
    
    // Verificar se o jogo foi resolvido
    if (isGameSolved(faces)) {
        std::cout << "Parabéns! Você resolveu o Elo Maluco!" << std::endl;
    }
    // std::cout << "Face vazia movida e XML atualizado!" << std::endl;
}

// Move face vazia para baixo
void moveFaceVaziaDown() {
    int vaziaIndex = -1;
    for (int i = 0; i < faces.size(); i++) {
        if (faces[i].isVazio) {
            vaziaIndex = i;
            break;
        }
    }

    if (vaziaIndex == -1 || vaziaIndex >= 12) return;

    // Trocar as faces
    std::swap(faces[vaziaIndex], faces[vaziaIndex + 4]);

    // Atualizar os códigos de posição
    faces[vaziaIndex].positionCode = cubeToPositionCode[vaziaIndex / 4];
    faces[vaziaIndex + 4].positionCode = "o";

    // Atualizar o XML
    updateXMLFromFaces();
    gameChanged = true; //<- importante para resetar o jogo caso o jogador queira ver a solução
    
    // Verificar se o jogo foi resolvido
    if (isGameSolved(faces)) {
        std::cout << "Parabéns! Você resolveu o Elo Maluco!" << std::endl;
    }
    // std::cout << "Face vazia movida e XML atualizado!" << std::endl;
}

void randomizeFaces() {
    std::vector<Face> newFaces;
    std::random_device rd;
    std::mt19937 g(rd());
    std::string colorT;

    // Criar vetores para cada tipo de face
    std::vector<Face> coloredFaces, whiteFaces, emptyFace;

    // Inicializar faces para cada cor
    for (const std::string& color : {"vm", "am", "vr"}) {
        if (color == "vm"){
            colorT = "red";
        } else if (color == "am") {
            colorT = "yellow";
        } else if (color == "vr") {
            colorT = "green";
        } else  {
            std::cerr << "Cor inválida encontrada: " << color << std::endl;
            continue;  // Pula esta iteração se a cor for inválida
        }
        try {
            std::string leftTexturePath = "./images/circulo_" + colorT + "_left.png";
            std::string linkTexturePath = "./images/circulo_" + colorT + "_link.png";
            std::string rightTexturePath = "./images/circulo_" + colorT + "_right.png";

            GLuint leftTextureID = loadTexture(leftTexturePath.c_str());
            GLuint linkTextureID = loadTexture(linkTexturePath.c_str());
            GLuint rightTextureID = loadTexture(rightTexturePath.c_str());

            if (leftTextureID == 0 || linkTextureID == 0 || rightTextureID == 0) {
                throw std::runtime_error("Falha ao carregar uma ou mais texturas para a cor " + colorT);
            }

            coloredFaces.push_back({color, "", "left", leftTextureID, false});
            coloredFaces.push_back({color, "", "link", linkTextureID, false});
            coloredFaces.push_back({color, "", "link", linkTextureID, false});
            coloredFaces.push_back({color, "", "right", rightTextureID, false});
        } catch (const std::exception& e) {
            std::cerr << "Erro ao criar face para a cor " << colorT << ": " << e.what() << std::endl;
        }
    }
    
    try { // Inicializar faces brancas
        GLuint whiteLeftID = loadTexture("./images/circulo_white_left.png");
        GLuint whiteLinkID = loadTexture("./images/circulo_white_link.png");
        GLuint whiteRightID = loadTexture("./images/circulo_white_right.png");

        if (whiteLeftID == 0 || whiteLinkID == 0 || whiteRightID == 0) {
            throw std::runtime_error("Falha ao carregar uma ou mais texturas brancas");
        }

        whiteFaces.push_back({"br", "", "left", whiteLeftID, false});
        whiteFaces.push_back({"br", "", "link", whiteLinkID, false});
        whiteFaces.push_back({"br", "", "right", whiteRightID, false});
    } catch (const std::exception& e) {
        std::cerr << "Erro ao criar faces brancas: " << e.what() << std::endl;
    }

    // Inicializar face vazia
    emptyFace.push_back({"vz", "o", "", 0, true});

    // Embaralhar todas as faces coloridas
    std::shuffle(coloredFaces.begin(), coloredFaces.end(), g);

    // Embaralhar faces brancas
    std::shuffle(whiteFaces.begin(), whiteFaces.end(), g);

    // Distribuir as faces
    for (int i = 0; i < 16; ++i) {
        if (i < 12 && !coloredFaces.empty()) {
            newFaces.push_back(coloredFaces[i % coloredFaces.size()]);
        } else if (i < 15 && !whiteFaces.empty()) {
            newFaces.push_back(whiteFaces[(i - 12) % whiteFaces.size()]);
        } else {
            newFaces.push_back(emptyFace[0]);
        }
    }

    // Embaralhar todas as faces
    std::shuffle(newFaces.begin(), newFaces.end(), g);

    // Atualizar as posições
    for (int i = 0; i < newFaces.size(); ++i) {
        int cubeIndex = i / 4;
        newFaces[i].positionCode = newFaces[i].isVazio ? "o" : cubeToPositionCode[cubeIndex];
    }

    // Atualizar o vetor global de faces
    faces = newFaces;

    // Atualizar o XML
    updateXMLFromFaces();
    gameChanged = true; //<- importante para resetar o jogo caso o jogador queira ver a solução

    std::cout << "Faces randomizadas!" << std::endl;
    glutPostRedisplay();
}

// Função para atualizar o XML a partir do vetor de faces
void updateXMLFromFaces() {
    std::vector<FaceData> faceDatas;
    for (const auto& face : faces) {
        FaceData faceData;
        faceData.colorCode = face.colorCode;
        faceData.positionCode = face.positionCode;
        faceData.textureType = face.textureType;
        faceDatas.push_back(faceData);
    }
    updateXML(faceDatas);
}

// Função para carregar as texturas das faces
GLuint loadTexture(const char *filename){
    int width, height, channels;
    unsigned char *image = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

    if (!image)
    {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    stbi_image_free(image);
    return textureID;
}

// Função de inicialização das texturas
void initTextures(){
    const char *textureFiles[] = {
        "./images/circulo_red_left.png", "./images/circulo_red_link.png", "./images/circulo_red_right.png",
        "./images/circulo_yellow_left.png", "./images/circulo_yellow_link.png", "./images/circulo_yellow_right.png",
        "./images/circulo_green_left.png", "./images/circulo_green_link.png", "./images/circulo_green_right.png",
        "./images/circulo_white_left.png", "./images/circulo_white_link.png", "./images/circulo_white_right.png"};

    for (int i = 0; i < 12; ++i)
    {
        textureIDs[i] = loadTexture(textureFiles[i]);
    }
}

// Função que inicializa as componentes de iluminação (A iluminação é controlada pela função display)
void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
}

// Define o material do objeto para a iluminação
void setMaterial() {
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
}


// Função para rotacionar o cubo (conjunto de 4 faces) selecionado
void rotateCube(int cubeIndex, float angle){

    if (cubeIndex < 0 || cubeIndex > 3 || cubeIndex == 1 || cubeIndex == 2) return;
    int rotations = static_cast<int>(std::round(angle / 90.0f)) % 4;
    if (rotations < 0) rotations += 4;

    int startIndex = cubeIndex * 4;
    std::vector<Face> cubeFaces(faces.begin() + startIndex, faces.begin() + startIndex + 4);

    for (int i = 0; i < rotations; ++i) {
        std::rotate(cubeFaces.rbegin(), cubeFaces.rbegin() + 1, cubeFaces.rend());
    }

    // Atualizar as posições
    for (int i = 0; i < 4; ++i) {
        cubeFaces[i].positionCode = cubeToPositionCode[cubeIndex];
        if (cubeFaces[i].isVazio) {
            cubeFaces[i].positionCode = "o";
        }
    }

    // Copiar de volta para faces
    std::copy(cubeFaces.begin(), cubeFaces.end(), faces.begin() + startIndex);

    updateXMLFromFaces();
    gameChanged = true; //<- importante para resetar o jogo caso o jogador queira ver a solução

    // Verificar se o jogo foi resolvido
    if (isGameSolved(faces)) {
        std::cout << "Parabéns! Você resolveu o Elo Maluco!" << std::endl;
        // Você pode adicionar aqui qualquer ação adicional que queira executar quando o jogo for resolvido
    }

    glutPostRedisplay();
    
}

// Verifica se o jogo foi resolvido
bool isGameSolved(const std::vector<Face>& faces){
    const std::vector<std::string> correctTextureOrder  = {"left", "link", "link", "right"};
    const std::vector<std::string> correctTextureOrderWhite  = {"left", "link", "right"};
    const std::vector<std::string> correctOrder = {"s", "m2", "m1", "i"};

    // Verificar cada lado do cubo (4 lados no total)
    for (int side = 0; side < 4; ++side) {
        std::string sideColor = "";
        std::vector<std::string> sideTextures;
        std::vector<std::string> sidePositions;
        
        // Coletar faces para este lado
        for (int i = 0; i < 4; ++i) {
            const auto& face = faces[side + i * 4];
            if (sideColor.empty() && face.colorCode != "vz") {
                sideColor = face.colorCode;
            } else if (face.colorCode != "vz" && face.colorCode != sideColor) {
                // Se encontrarmos uma cor diferente (que não seja vazio), o lado não está resolvido
                // std::cout << "Cor errada na coluna: " << sideColor << std::endl;
                return false;
            }
            if (face.colorCode != "vz") {
                sideTextures.push_back(face.textureType);
                sidePositions.push_back(face.positionCode);
            }
        }
        
        // Verificar a ordem das posições
        if (sideColor != "br"){
            if (!std::is_permutation(sidePositions.begin(), sidePositions.end(), correctOrder.begin(), correctOrder.end())) {
                // std::cout << "Ordem errada de Código: " << sideColor << std::endl;
                // for (const auto& position : sidePositions){
                //     std::cout << position << std::endl;
                // }
                return false;
            }
        }
        
        // Verificar a ordem das texturas
        if (sideColor == "br") {
            if (sideTextures.size() != 3 || !std::is_permutation(sideTextures.begin(), sideTextures.end(), correctTextureOrderWhite.begin(), correctTextureOrderWhite.end())) {
                //std::cout << "Ordem errada: " << sideColor << std::endl;
                return false;
            }
        } else if (!sideTextures.empty()) {
            if (sideTextures.size() != 4 || !std::is_permutation(sideTextures.begin(), sideTextures.end(), correctTextureOrder.begin(), correctTextureOrder.end())) {
                //std::cout << "Ordem errada: " << sideColor << std::endl;
                // for (const auto& texture : sideTextures){
                //     std::cout << texture << std::endl;
                // }
                return false;
            }
        }
    }
    // std::cout << "Jogo resolvido!" << std::endl;
    gameSolved = true;
    return true;
}

// Função para definir os atalhos do teclado
void keyboard(unsigned char key, int x, int y){
    const float moveSpeed = 0.1f;
    const float rotateSpeed = 2.0f;

    switch (key){
    case 'r':
        if (selectedCube != -1){
            rotateCube(selectedCube, 90.0f);
        }
        break;
    case 't':
        if (selectedCube != -1){
            rotateCube(selectedCube, -90.0f);
        }
        break;
    case 'n':
        moveFaceVaziaDown();
        break;
    case 'm':
        moveFaceVaziaUp();
        break;
    case '1':
        selectedCube = 3;
        selectedFace = 0;
        break;
    case '2':
        selectedCube = 2;
        selectedFace = 0;
        break;
    case '3':
        selectedCube = 1;
        selectedFace = 0;
        break;
    case '4':
        selectedCube = 0;
        selectedFace = 0;
        break;
        // case '5': selectedFace = (selectedFace + 1) % 4; break;
    }

    glutPostRedisplay();
}

// Função para definir os atalhos do teclado especial
void specialKeyboard(int key, int x, int y)
{
    const float rotateAngle = 5.0f;

    switch (key)
    {
    case GLUT_KEY_LEFT:
        objectRotation.rotateY(-rotateAngle);
        break;
    case GLUT_KEY_RIGHT:
        objectRotation.rotateY(rotateAngle);
        break;
    }

    glutPostRedisplay();
}

// Função para definir o comportamento do mouse
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        int windowY = windowHeight - y; // Invert Y coordinate

        for (const auto &btn : buttons)
        {
            if (x >= btn.x && x <= btn.x + btn.width &&
                windowY >= btn.y && windowY <= btn.y + btn.height)
            {
                if (btn.callback)
                {
                    btn.callback();
                }
                break;
            }
        }
    }
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouseLeftDown = true;
            mouseX = x;
            mouseY = y;
        }
        else if (state == GLUT_UP)
        {
            mouseLeftDown = false;
        }
    }
}

// Função para definir o movimento(girar) do objeto pelo mouse e pelo teclado
void motion(int x, int y)
{
    if (mouseLeftDown)
    {
        int deltaX = x - mouseX;
        int deltaY = y - mouseY;

        const int threshold = 2;
        if (std::abs(deltaX) > threshold || std::abs(deltaY) > threshold)
        {
            objectRotation.rotate(static_cast<float>(deltaX), 0.0f);
            mouseX = x;
            mouseY = y;
            glutPostRedisplay();
        }
    }
}

void restart(){
    gameSolved = false;
    gameChanged = false;
    faces = facesOriginals;
    updateXMLFromFaces();
    actionManager.resetActionIndex();
}