#include "program.h"
using namespace std;
struct date
{
    int16_t year;
    int8_t month;
    int8_t day;
};

struct person
{
    uint32_t nextNode;
    uint32_t prevNode;
    int8_t role;
    char id[8];
    char name[32];
    date birthday;
    date join_date;
    char phone[15];
    char cccd[15];
};

struct listStudent
{
    uint32_t head;
    uint32_t tail;
    uint32_t size;
};

struct listTeacher
{
    uint32_t head;
    uint32_t tail;
    uint32_t size;
};

uint32_t findAddrEmptySpace(fstream &data)
{
    uint32_t sizeFile;
    data.seekg(0, std::ios::end);
    sizeFile = data.tellg();
    uint32_t role = 0;
    data.seekg(LENGTH_HEADER + sizeof(uint32_t) * 2, std::ios::beg);
    while (true)
    {
        data.read((char *)&role, sizeof(int8_t));
        if (role <= 0)
        {
            data.seekg(((uint32_t)data.tellg()) - (sizeof(int8_t) + sizeof(uint32_t) * 2), std::ios::beg);
            break;
        }
        if (role + sizeof(person) >= sizeFile)
        {
            data.seekg(0, std::ios::end);
            break;
        }
        data.seekg(role + sizeof(person));
    }
    return data.tellg();
}

BYTE *sha256_test(BYTE *text, int length)
{
    BYTE *buf = new BYTE[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, text, length);
    sha256_final(&ctx, buf);
    return buf;
}

class cryptocurrency
{
private:
    BYTE key[32] = {0x00};
    WORD key_schedule[60];

public:
    void initPassword(BYTE key[32])
    {
        memcpy(this->key, key, 32);
        aes_key_setup(key, key_schedule, 256);
    }

    char *encrypt(string plantext)
    {
        int padding = 16 - plantext.length() % 16;
        BYTE *buf = new BYTE[plantext.length() + padding];
        memcpy(buf, plantext.c_str(), plantext.length());
        buf[plantext.length()] = 0x01;
        for (int i = plantext.length() + 1; i < plantext.length() + padding; i++)
        {
            buf[i] = 0x00;
        }
        // using ecb mode, buf.length equal 16
        aes_encrypt(buf, buf, key_schedule, 256);
        return (char *)buf;
    }

    string decrypt(char *ciphertext)
    {
        aes_decrypt((BYTE *)ciphertext, (BYTE *)ciphertext, key_schedule, 256);
        string plaintext = ciphertext;
        int padding = 0;
        for (int i = plaintext.length() - 1; i >= 0; i--)
        {
            if (plaintext[i] == 0x01)
            {
                padding = plaintext.length() - i - 1;
                break;
            }
        }
        plaintext.erase(plaintext.length() - padding, padding);
        return plaintext;
    }
};

cryptocurrency crypto;

void initDataBase(fstream &data)
{
    // if data.bin is not exist, create it and write listStudent and listTeacher to it with size = 0, head = 0, tail = 0
    if (!data.is_open())
    {
        data.open("data.bin", std::ios::out | std::ios::binary);
        printf("data.bin is not exist, create new data.bin\n");
        // require password to encrypt data.bin
        printf("password: ");
        string password;
        cin >> password;
        printf("repassword: ");
        string repassword;
        cin >> repassword;
        if (password != repassword)
        {
            printf("password not match\n");
            return;
        }
        // encrypt password to key
        BYTE *Key = sha256_test((BYTE *)password.c_str(), password.length());
        // 32 bytes first is Hash(key)
        BYTE *HashKey = sha256_test(Key, 32);
        // write Hash(key) to data.bin
        data.write((char *)HashKey, 32);
        listStudent ls;
        ls.head = 0;
        ls.tail = 0;
        ls.size = 0;
        data.write((char *)&ls, sizeof(listStudent));
        listTeacher lt;
        lt.head = 0;
        lt.tail = 0;
        lt.size = 0;
        data.write((char *)&lt, sizeof(listTeacher));
        data.close();
        // delete Key, HashKey
        delete[] Key;
        delete[] HashKey;
        printf("create new data.bin success\n\n");
        data.open("data.bin", std::ios::in | std::ios::out | std::ios::binary);
    }
    printf("welcome to student management system\n");
}

void login(fstream &data)
{
    BYTE *trueHashKey = new BYTE[32];
    BYTE *Key;
    BYTE *HashKey;
    data.read((char *)trueHashKey, 32);
    // login
    while (true)
    {
        printf("password: ");
        string password;
        cin >> password;
        Key = sha256_test((BYTE *)password.c_str(), password.length());
        HashKey = sha256_test(Key, 32);
        if (memcmp(trueHashKey, HashKey, 32) != 0)
        {
            printf("password is wrong\n");
        }
        else
        {
            break;
        }
    }
    crypto.initPassword(Key);
    delete[] Key;
    delete[] HashKey;
    delete[] trueHashKey;
    printf("login success\n");
}

void case1(fstream &data, listStudent ls)
{
    printf("list students\n");
    if (ls.size == 0)
    {
        printf("list students is empty\n");
    }
    else
    {
        person *p = new person;
        uint32_t pos = ls.head;
        while (pos != 0)
        {
            data.seekg(pos);
            data.read((char *)p, sizeof(person));
            printf("id: %s\n", p->id);
            printf("name: %s\n", p->name);
            printf("birthday: %d/%d/%d\n", p->birthday.day, p->birthday.month, p->birthday.year);
            printf("join date: %d/%d/%d\n", p->join_date.day, p->join_date.month, p->join_date.year);
            printf("phone: %s\n", crypto.decrypt((char *)p->phone).c_str());
            printf("cccd: %s\n", crypto.decrypt((char *)p->cccd).c_str());
            printf("\n");
            pos = p->nextNode;
        }
        delete p;
    }
}

void case2(fstream &data, listTeacher lt)
{
    printf("list teachers\n");
    if (lt.size == 0)
    {
        printf("list teachers is empty\n");
    }
    else
    {
        person *p = new person;
        uint32_t pos = lt.head;
        while (pos != 0)
        {
            data.seekg(pos);
            data.read((char *)p, sizeof(person));
            printf("id: %s\n", p->id);
            printf("name: %s\n", p->name);
            printf("birthday: %d/%d/%d\n", p->birthday.day, p->birthday.month, p->birthday.year);
            printf("join date: %d/%d/%d\n", p->join_date.day, p->join_date.month, p->join_date.year);
            printf("phone: %s\n", crypto.decrypt((char *)p->phone).c_str());
            printf("cccd: %s\n", crypto.decrypt((char *)p->cccd).c_str());
            printf("\n");
            pos = p->nextNode;
        }
        delete p;
    }
}

void case3(fstream &data, listStudent &ls)
{
    printf("add student\n");
    person *p = new person;
    printf("id: ");
    cin >> p->id;
    printf("name: ");
    cin.ignore();
    cin.getline(p->name, 32);
    printf("birthday: ");
    cin >> p->birthday.day >> p->birthday.month >> p->birthday.year;
    printf("join date: ");
    cin >> p->join_date.day >> p->join_date.month >> p->join_date.year;
    printf("phone: ");
    cin.ignore();
    cin.getline(p->phone, 15);
    printf("cccd: ");
    cin.getline(p->cccd, 15);
    p->role = 0;
    p->nextNode = 0;
    p->prevNode = 0;
    // encrypt phone and cccd
    strcpy(p->phone, crypto.encrypt(p->phone));
    strcpy(p->cccd, crypto.encrypt(p->cccd));
    // write to data.bin
    uint32_t pos = findAddrEmptySpace(data);
    // update listStudent
    if (ls.size == 0)
    {
        ls.head = pos;
        ls.tail = pos;
        data.seekg(0, pos);
        data.write((char *)p, sizeof(person));
    }
    else
    {
        person *p1 = new person;
        uint32_t pos1 = ls.head;
        while (pos1 != 0)
        {
            data.seekg(pos1);
            data.read((char *)p1, sizeof(person));
            if (strcmp(p->id, p1->id) < 0)
            {
                break;
            }
            pos1 = p1->nextNode;
        }
        if (pos1 == 0)
        {
            p1->nextNode = pos;
            data.seekg(ls.tail);
            data.write((char *)p1, sizeof(person));
            p->prevNode = ls.tail;
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            ls.tail = pos;
        }
        else
        {
            person *p2 = new person;
            data.seekg(p1->prevNode);
            data.read((char *)p2, sizeof(person));
            p2->nextNode = pos;
            data.seekg(p1->prevNode);
            data.write((char *)p2, sizeof(person));
            p->prevNode = p1->prevNode;
            p->nextNode = pos1;
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            p1->prevNode = pos;
            data.seekg(pos1);
            data.write((char *)p1, sizeof(person));
            delete p2;
        }
        delete p1;
    }
    ls.size++;
    data.seekg(32);
    data.write((char *)&ls, sizeof(listStudent));
    delete p;
}

void case4(fstream &data, listTeacher &lt)
{
    printf("add teacher\n");
    person *p = new person;
    printf("id: ");
    cin >> p->id;
    printf("name: ");
    cin.ignore();
    cin.getline(p->name, 32);
    printf("birthday: ");
    cin >> p->birthday.day >> p->birthday.month >> p->birthday.year;
    printf("join date: ");
    cin >> p->join_date.day >> p->join_date.month >> p->join_date.year;
    printf("phone: ");
    cin.ignore();
    cin.getline(p->phone, 15);
    printf("cccd: ");
    cin.getline(p->cccd, 15);
    p->role = 1;
    p->nextNode = 0;
    p->prevNode = 0;
    // encrypt phone and cccd
    strcpy(p->phone, crypto.encrypt(p->phone));
    strcpy(p->cccd, crypto.encrypt(p->cccd));
    // write to data.bin
    uint32_t pos = findAddrEmptySpace(data);
    // update listTeacher
    if (lt.size == 0)
    {
        lt.head = pos;
        lt.tail = pos;
        data.seekg(32 + sizeof(listStudent));
        data.write((char *)&lt, sizeof(listTeacher));
        data.seekg(pos);
        data.write((char *)p, sizeof(person));
    }
    else
    {
        person *p1 = new person;
        uint32_t pos1 = lt.head;
        while (pos1 != 0)
        {
            data.seekg(pos1);
            data.read((char *)p1, sizeof(person));
            if (strcmp(p->id, p1->id) < 0)
            {
                break;
            }
            pos1 = p1->nextNode;
        }
        if (pos1 == 0)
        {
            p1->nextNode = pos;
            data.seekg(lt.tail);
            data.write((char *)p1, sizeof(person));
            p->prevNode = lt.tail;
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            lt.tail = pos;
        }
        else
        {
            person *p2 = new person;
            data.seekg(p1->prevNode);
            data.read((char *)p2, sizeof(person));
            p2->nextNode = pos;
            data.seekg(p1->prevNode);
            data.write((char *)p2, sizeof(person));
            p->prevNode = p1->prevNode;
            p->nextNode = pos1;
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            p1->prevNode = pos;
            data.seekg(pos1);
            data.write((char *)p1, sizeof(person));
            delete p2;
        }
        delete p1;
    }
    lt.size++;
    data.seekg(32 + sizeof(listStudent));
    data.write((char *)&lt, sizeof(listTeacher));
    delete p;
}

int main()
{
    fstream data("data.bin", std::ios::in | std::ios::out | std::ios::binary);
    initDataBase(data);
    login(data);
    listStudent ls;
    data.read((char *)&ls, sizeof(listStudent));
    listTeacher lt;
    data.read((char *)&lt, sizeof(listTeacher));
    // main loop
    while (true)
    {
        printf("1. show list students\n");
        printf("2. show list teachers\n");
        printf("3. add student\n");
        printf("4. add teacher\n");
        printf("5. delete student\n");
        printf("6. delete teacher\n");
        printf("7. edit student\n");
        printf("8. edit teacher\n");
        printf("9. exit\n");
        printf("select: ");
        int select;
        cin >> select;
        switch (select)
        {
        case 1:
        {
            case1(data, ls);
            break;
        }
        case 2:
        {
            case2(data, lt);
            break;
        }
        case 3:
        {
            case3(data, ls);
            break;
        }
        case 4:
        {
            case4(data, lt);
            break;
        }
        }
        data.close();
        return 0;
    }
}