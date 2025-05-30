#include <iostream>
#include <cstring>
using namespace std;

struct Video {
    char title[100];
    int duration;
    char status[20]; 
    Video *left, *right;
};

struct PlaylistNode {
    Video* video;
    PlaylistNode* next;
};

struct HistoryNode {
    Video* video;
    HistoryNode* next;
};

struct Action {
    char type[20]; 
    Video* video;
    Action* next;
};

Video* root = NULL;
PlaylistNode* playlistHead = NULL;
HistoryNode* historyHead = NULL;
Action* actionStack = NULL;

Video* createVideo(const char* title, int duration) {
    Video* v = new Video;
    strcpy(v->title, title);
    v->duration = duration;
    strcpy(v->status, "tersedia");
    v->left = v->right = NULL;
    return v;
}

void inputVideo(Video*& node, Video* v) {
    if (node == NULL) {
        node = v;
        return;
    }
    if (strcmp(v->title, node->title) < 0)
        inputVideo(node->left, v);
    else if (strcmp(v->title, node->title) > 0)
        inputVideo(node->right, v);
}

Video* findVideo(Video* node, const char* title) {
    if (!node) return NULL;
    if (strcmp(title, node->title) == 0) return node;
    if (strcmp(title, node->title) < 0)
        return findVideo(node->left, title);
    return findVideo(node->right, title);
}

void inorder(Video* node) {
    if (!node) return;
    inorder(node->left);
    cout << "- " << node->title << " (" << node->duration << " menit), status: " << node->status << "\n";
    inorder(node->right);
}

void pushAction(const char* type, Video* v) {
    Action* a = new Action;
    strcpy(a->type, type);
    a->video = v;
    a->next = actionStack;
    actionStack = a;
}

Action* popAction() {
    if (!actionStack) return NULL;
    Action* a = actionStack;
    actionStack = actionStack->next;
    return a;
}

void addToPlaylist(Video* v) {
    if (strcmp(v->status, "tersedia") != 0) {
        cout << "Video sudah dalam playlist atau sedang diputar.\n";
        return;
    }
    PlaylistNode* newNode = new PlaylistNode{v, NULL};
    if (!playlistHead) {
        playlistHead = newNode;
        strcpy(v->status, "sedang diputar");
    } else {
        PlaylistNode* temp = playlistHead;
        while (temp->next) temp = temp->next;
        temp->next = newNode;
        strcpy(v->status, "dalam antrean");
    }
    pushAction("playlist", v);
    cout << "Video ditambahkan ke playlist.\n";
}

void playNext() {
    if (!playlistHead) {
        cout << "Playlist kosong.\n";
        return;
    }
    Video* nowPlaying = playlistHead->video;
    strcpy(nowPlaying->status, "tersedia");

    HistoryNode* newHistory = new HistoryNode{nowPlaying, historyHead};
    historyHead = newHistory;

    playlistHead = playlistHead->next;
    if (playlistHead) {
        strcpy(playlistHead->video->status, "sedang diputar");
    }
    pushAction("play", nowPlaying);
    cout << "Video telah selesai diputar dan masuk ke riwayat.\n";
}

void showHistory() {
    cout << "Riwayat tontonan:\n";
    HistoryNode* temp = historyHead;
    while (temp) {
        cout << "- " << temp->video->title << "\n";
        temp = temp->next;
    }
}

void deleteVideo(Video*& node, const char* title) {
    if (!node) return;
    if (strcmp(title, node->title) < 0)
        deleteVideo(node->left, title);
    else if (strcmp(title, node->title) > 0)
        deleteVideo(node->right, title);
    else {
        Video* target = node;
        if (node->left && node->right) {
            Video* succ = node->right;
            while (succ->left) succ = succ->left;
            strcpy(node->title, succ->title);
            node->duration = succ->duration;
            strcpy(node->status, succ->status);
            deleteVideo(node->right, succ->title);
        } else {
            node = (node->left) ? node->left : node->right;
            pushAction("delete", target);
        }
    }
}

void undo() {
    Action* last = popAction();
    if (!last) {
        cout << "Tidak ada tindakan yang bisa di-undo.\n";
        return;
    }
    if (strcmp(last->type, "add") == 0) {
        deleteVideo(root, last->video->title);
        cout << "Undo tambah video.\n";
    } else if (strcmp(last->type, "delete") == 0) {
        inputVideo(root, last->video);
        cout << "Undo hapus video.\n";
    } else if (strcmp(last->type, "playlist") == 0) {
        PlaylistNode** temp = &playlistHead;
        while (*temp && (*temp)->video != last->video)
            temp = &(*temp)->next;
        if (*temp) {
            PlaylistNode* toDelete = *temp;
            *temp = (*temp)->next;
            delete toDelete;
        }
        strcpy(last->video->status, "tersedia");
        cout << "Undo tambah ke playlist.\n";
    } else if (strcmp(last->type, "play") == 0) {
        Video* v = last->video;
        if (playlistHead) {
            strcpy(playlistHead->video->status, "dalam antrean");
        }
        PlaylistNode* newNode = new PlaylistNode{v, playlistHead};
        playlistHead = newNode;
        strcpy(v->status, "sedang diputar");

        if (historyHead && historyHead->video == v) {
            HistoryNode* old = historyHead;
            historyHead = historyHead->next;
            delete old;
        }
        cout << "Undo tonton video.\n";
    }
    delete last;
}

void menu() {
    while (true) {
        cout << "\n===== IDLIX Tube Menu =====\n";
        cout << "1. Tambah Video\n";
        cout << "2. Tampilkan Daftar Video\n";
        cout << "3. Tambah ke Playlist\n";
        cout << "4. Tonton Video\n";
        cout << "5. Lihat Riwayat Tontonan\n";
        cout << "6. Hapus Video\n";
        cout << "7. Undo Tindakan Terakhir\n";
        cout << "0. Keluar\n";
        cout << "===========================\n";
        cout << "Pilihan Anda: ";

        int choice;
        cin >> choice;
        cin.ignore(); 

        char title[100];
        int duration;

        switch (choice) {
            case 1:
                cout << "\n>> Tambah Video\n";
                cout << "Judul: ";
                cin.getline(title, 100);
                if (findVideo(root, title)) {
                    cout << "Judul video sudah ada!\n";
                    break;
                }
                cout << "Durasi (menit): ";
                cin >> duration;
                cin.ignore();
                {
                    Video* v = createVideo(title, duration);
                    inputVideo(root, v);
                    pushAction("add", v);
                    cout << "Video berhasil ditambahkan!\n";
                }
                break;

            case 2:
                cout << "\n>> Daftar Video:\n";
                inorder(root);
                cout << "\nIngin mencari video? (y/t): ";
                char cari;
                cin >> cari;
                cin.ignore();
                if (cari == 'y' || cari == 'Y') {
                    cout << "Masukkan judul: ";
                    cin.getline(title, 100);
                    Video* found = findVideo(root, title);
                    if (found) {
                        cout << "Ditemukan: " << found->title << ", durasi: " << found->duration << " menit, status: " << found->status << "\n";
                    } else {
                        cout << "Video tidak ditemukan.\n";
                    }
                }
                break;

            case 3:
                cout << "\n>> Tambah ke Playlist\n";
                cout << "Judul video: ";
                cin.getline(title, 100);
                {
                    Video* v = findVideo(root, title);
                    if (v) {
                        addToPlaylist(v);
                    } else {
                        cout << "Video tidak ditemukan!\n";
                    }
                }
                break;

            case 4:
                cout << "\n>> Tonton Video\n";
                playNext();
                break;

            case 5:
                cout << "\n>> Riwayat Tontonan\n";
                showHistory();
                break;

            case 6:
                cout << "\n>> Hapus Video\n";
                cout << "Judul video: ";
                cin.getline(title, 100);
                {
                    Video* v = findVideo(root, title);
                    if (!v) {
                        cout << "Video tidak ditemukan.\n";
                        break;
                    }
                    if (strcmp(v->status, "sedang diputar") == 0 || strcmp(v->status, "dalam antrean") == 0) {
                        cout << "Video sedang " << v->status << ". Yakin ingin hapus? (y/t): ";
                        char confirm;
                        cin >> confirm;
                        cin.ignore();
                        if (confirm != 'y' && confirm != 'Y') break;
                    }
                    deleteVideo(root, title);
                    cout << "Video berhasil dihapus.\n";
                }
                break;

            case 7:
                cout << "\n>> Undo Tindakan\n";
                undo();
                break;

            case 0:
                cout << "\nTerima kasih telah menggunakan IDLIX Tube!\n";
                return;

            default:
                cout << "Pilihan tidak valid. Silakan coba lagi.\n";
                break;
        }
    }
}

int main() {
    menu();
    return 0;
}
