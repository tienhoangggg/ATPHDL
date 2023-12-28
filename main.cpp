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
    char id[10];
    char name[32];
    date birthday;
    date join_date;
    char phone[16];
    char cccd[16];
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
    uint8_t role;
    if (sizeFile == LENGTH_HEADER)
    {
        return LENGTH_HEADER;
    }
    data.seekg(LENGTH_HEADER + sizeof(uint32_t) * 2);
    while (true)
    {
        data.read((char *)&role, sizeof(int8_t));
        if (role <= 0)
        {
            data.seekg(((uint32_t)data.tellg()) - (sizeof(int8_t) + sizeof(uint32_t) * 2));
            break;
        }
        if (((uint32_t)data.tellg()) + sizeof(person) > sizeFile)
        {
            data.seekg(0, std::ios::end);
            break;
        }
        data.seekg(((uint32_t)data.tellg()) + sizeof(person));
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
        char *buf = new char[16];
        aes_decrypt((BYTE *)ciphertext, (BYTE *)buf, key_schedule, 256);
        int padding;
        for (int i = 0; i < 16; i++)
        {
            if (buf[i] == 0x01)
            {
                padding = i;
                break;
            }
        }
        string plaintext;
        plaintext.assign(buf, padding);
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

void case1(fstream &data)
{
    printf("list students\n- - - - - - - - - - - - - -\n");
    listStudent *ls = new listStudent;
    data.seekg(32);
    data.read((char *)ls, sizeof(listStudent));
    printf("size: %d\n", ls->size);
    if (ls->size == 0)
    {
        printf("list students is empty\n");
    }
    else
    {
        person *p = new person;
        uint32_t pos = ls->head;
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
    delete ls;
}

void case2(fstream &data)
{
    printf("list teachers\n- - - - - - - - - - - - - -\n");
    listTeacher *lt = new listTeacher;
    data.seekg(32 + sizeof(listStudent));
    data.read((char *)lt, sizeof(listTeacher));
    printf("size: %d\n", lt->size);
    if (lt->size == 0)
    {
        printf("list teachers is empty\n");
    }
    else
    {
        person *p = new person;
        uint32_t pos = lt->head;
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
    delete lt;
}

void case3(fstream &data)
{
    printf("add student\n- - - - - - - - - - - - - -\n");
    listStudent* ls = new listStudent;
    data.seekg(32);
    data.read((char *)ls, sizeof(listStudent));
    person *p = new person;
    printf("id: ");
    cin >> p->id;
    printf("name: ");
    cin.ignore();
    cin.getline(p->name, 32);
    printf("birthday: \n");
    int temp;
    printf("    day: ");
    cin >> temp;
    p->birthday.day = temp;
    printf("    month: ");
    cin >> temp;
    p->birthday.month = temp;
    printf("    year: ");
    cin >> temp;
    p->birthday.year = temp;
    printf("join date: \n");
    printf("    day: ");
    cin >> temp;
    p->join_date.day = temp;
    printf("    month: ");
    cin >> temp;
    p->join_date.month = temp;
    printf("    year: ");
    cin >> temp;
    p->join_date.year = temp;
    printf("phone: ");
    cin.ignore();
    string phone;
    getline(cin, phone);
    printf("cccd: ");
    string cccd;
    getline(cin, cccd);
    p->role = 2;
    p->nextNode = 0;
    p->prevNode = 0;
    // encrypt phone and cccd
    strcpy(p->phone, crypto.encrypt(phone));
    strcpy(p->cccd, crypto.encrypt(cccd));
    uint32_t pos = findAddrEmptySpace(data);
    // update listStudent
    if (ls->size == 0)
    {
        ls->head = pos;
        ls->tail = pos;
        data.seekg(pos);
        data.write((char *)p, sizeof(person));
    }
    else
    {
        person *p1 = new person;
        uint32_t pos1 = ls->head;
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
            data.seekg(ls->tail);
            data.write((char *)p1, sizeof(person));
            p->prevNode = ls->tail;
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            ls->tail = pos;
        }
        else
        if (pos1 == ls->head)
        {
            p->nextNode = pos1;
            p1->prevNode = pos;
            data.seekg(pos1);
            data.write((char *)p1, sizeof(person));
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            ls->head = pos;
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
    ls->size++;
    data.seekg(32);
    data.write((char *)ls, sizeof(listStudent));
    delete p;
    delete ls;
    printf("add success\n");
}

void case4(fstream &data)
{
    printf("add teacher\n- - - - - - - - - - - - - -\n");
    listTeacher *lt = new listTeacher;
    data.seekg(32 + sizeof(listStudent));
    data.read((char *)lt, sizeof(listTeacher));
    person *p = new person;
    printf("id: ");
    cin >> p->id;
    printf("name: ");
    cin.ignore();
    cin.getline(p->name, 32);
    printf("birthday: \n");
    int temp;
    printf("    day: ");
    cin >> temp;
    p->birthday.day = temp;
    printf("    month: ");
    cin >> temp;
    p->birthday.month = temp;
    printf("    year: ");
    cin >> temp;
    p->birthday.year = temp;
    printf("join date: \n");
    printf("    day: ");
    cin >> temp;
    p->join_date.day = temp;
    printf("    month: ");
    cin >> temp;
    p->join_date.month = temp;
    printf("    year: ");
    cin >> temp;
    p->join_date.year = temp;
    printf("phone: ");
    cin.ignore();
    string phone;
    getline(cin, phone);
    printf("cccd: ");
    string cccd;
    getline(cin, cccd);
    p->role = 1;
    p->nextNode = 0;
    p->prevNode = 0;
    // encrypt phone and cccd
    strcpy(p->phone, crypto.encrypt(phone));
    strcpy(p->cccd, crypto.encrypt(cccd));
    uint32_t pos = findAddrEmptySpace(data);
    // update listTeacher
    if (lt->size == 0)
    {
        lt->head = pos;
        lt->tail = pos;
        data.seekg(32 + sizeof(listStudent));
        data.write((char *)&lt, sizeof(listTeacher));
        data.seekg(pos);
        data.write((char *)p, sizeof(person));
    }
    else
    {
        person *p1 = new person;
        uint32_t pos1 = lt->head;
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
            data.seekg(lt->tail);
            data.write((char *)p1, sizeof(person));
            p->prevNode = lt->tail;
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            lt->tail = pos;
        }
        if (pos1 == lt->head)
        {
            p->nextNode = pos1;
            p1->prevNode = pos;
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            data.seekg(pos1);
            data.write((char *)p1, sizeof(person));
            lt->head = pos;
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
    lt->size++;
    data.seekg(32 + sizeof(listStudent));
    data.write((char *)lt, sizeof(listTeacher));
    delete p;
    delete lt;
    printf("add success\n");
}

void case5(fstream &data)
{
    printf("delete student\n- - - - - - - - - - - - - -\n");
    listStudent *ls = new listStudent;
    data.seekg(32);
    data.read((char *)ls, sizeof(listStudent));
    if (ls->size == 0)
    {
        printf("list students is empty\n");
    }
    else
    {
        printf("id: ");
        char id[8];
        cin >> id;
        person *p = new person;
        uint32_t pos = ls->head;
        while (pos != 0)
        {
            data.seekg(pos);
            data.read((char *)p, sizeof(person));
            if (strcmp(id, p->id) == 0)
            {
                break;
            }
            pos = p->nextNode;
        }
        if (pos == 0)
        {
            printf("id is not exist\n");
        }
        else
        {
            if (pos == ls->head)
            {
                ls->head = p->nextNode;
                person *p1 = new person;
                data.seekg(ls->head);
                data.read((char *)p1, sizeof(person));
                p1->prevNode = 0;
                data.seekg(ls->head);
                data.write((char *)p1, sizeof(person));
                delete p1;
            }
            else
            {
                person *p1 = new person;
                data.seekg(p->prevNode);
                data.read((char *)p1, sizeof(person));
                p1->nextNode = p->nextNode;
                data.seekg(p->prevNode);
                data.write((char *)p1, sizeof(person));
                delete p1;
            }
            if (pos == ls->tail)
            {
                ls->tail = p->prevNode;
                person *p2 = new person;
                data.seekg(ls->tail);
                data.read((char *)p2, sizeof(person));
                p2->nextNode = 0;
                data.seekg(ls->tail);
                data.write((char *)p2, sizeof(person));
                delete p2;
            }
            else
            {
                person *p2 = new person;
                data.seekg(p->nextNode);
                data.read((char *)p2, sizeof(person));
                p2->prevNode = p->prevNode;
                data.seekg(p->nextNode);
                data.write((char *)p2, sizeof(person));
                delete p2;
            }
            ls->size--;
            data.seekg(32);
            data.write((char *)ls, sizeof(listStudent));
            p->role = -p->role;
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            printf("delete success\n");
        }
        delete p;
    }
    delete ls;
}

void case6(fstream &data)
{
    printf("delete teacher\n- - - - - - - - - - - - - -\n");
    listTeacher *lt = new listTeacher;
    data.seekg(32 + sizeof(listStudent));
    data.read((char *)lt, sizeof(listTeacher));
    if (lt->size == 0)
    {
        printf("list teachers is empty\n");
    }
    else
    {
        printf("id: ");
        char id[8];
        cin >> id;
        person *p = new person;
        uint32_t pos = lt->head;
        while (pos != 0)
        {
            data.seekg(pos);
            data.read((char *)p, sizeof(person));
            if (strcmp(id, p->id) == 0)
            {
                break;
            }
            pos = p->nextNode;
        }
        if (pos == 0)
        {
            printf("id is not exist\n");
        }
        else
        {
            if (pos == lt->head)
            {
                lt->head = p->nextNode;
                person *p1 = new person;
                data.seekg(lt->head);
                data.read((char *)p1, sizeof(person));
                p1->prevNode = 0;
                data.seekg(lt->head);
                data.write((char *)p1, sizeof(person));
                delete p1;
            }
            else
            {
                person *p1 = new person;
                data.seekg(p->prevNode);
                data.read((char *)p1, sizeof(person));
                p1->nextNode = p->nextNode;
                data.seekg(p->prevNode);
                data.write((char *)p1, sizeof(person));
                delete p1;
            }
            if (pos == lt->tail)
            {
                lt->tail = p->prevNode;
                person *p2 = new person;
                data.seekg(lt->tail);
                data.read((char *)p2, sizeof(person));
                p2->nextNode = 0;
                data.seekg(lt->tail);
                data.write((char *)p2, sizeof(person));
                delete p2;
            }
            else
            {
                person *p2 = new person;
                data.seekg(p->nextNode);
                data.read((char *)p2, sizeof(person));
                p2->prevNode = p->prevNode;
                data.seekg(p->nextNode);
                data.write((char *)p2, sizeof(person));
                delete p2;
            }
            lt->size--;
            data.seekg(32 + sizeof(listStudent));
            data.write((char *)lt, sizeof(listTeacher));
            p->role = -p->role;
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            printf("delete success\n");
        }
        delete p;
    }
    delete lt;
}

void case7(fstream &data)
{
    printf("edit student\n- - - - - - - - - - - - - -\n");
    listStudent *ls = new listStudent;
    data.seekg(32);
    data.read((char *)ls, sizeof(listStudent));
    if (ls->size == 0)
    {
        printf("list students is empty\n");
    }
    else
    {
        printf("id: ");
        char id[8];
        cin >> id;
        person *p = new person;
        uint32_t pos = ls->head;
        while (pos != 0)
        {
            data.seekg(pos);
            data.read((char *)p, sizeof(person));
            if (strcmp(id, p->id) == 0)
            {
                break;
            }
            pos = p->nextNode;
        }
        if (pos == 0)
        {
            printf("id is not exist\n");
        }
        else
        {
            printf("name: ");
            cin.ignore();
            cin.getline(p->name, 32);
            int temp;
            printf("birthday: \n");
            printf("    day: ");
            cin >> temp;
            p->birthday.day = temp;
            printf("    month: ");
            cin >> temp;
            p->birthday.month = temp;
            printf("    year: ");
            cin >> temp;
            p->birthday.year = temp;
            printf("join date: \n");
            printf("    day: ");
            cin >> temp;
            p->join_date.day = temp;
            printf("    month: ");
            cin >> temp;
            p->join_date.month = temp;
            printf("    year: ");
            cin >> temp;
            p->join_date.year = temp;
            printf("phone: ");
            cin.ignore();
            string phone;
            getline(cin, phone);
            printf("cccd: ");
            string cccd;
            getline(cin, cccd);
            // encrypt phone and cccd
            strcpy(p->phone, crypto.encrypt(phone));
            strcpy(p->cccd, crypto.encrypt(cccd));
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            printf("edit success\n");
        }
        delete p;
    }
    delete ls;
}

void case8(fstream &data)
{
    printf("edit teacher\n- - - - - - - - - - - - - -\n");
    listTeacher *lt = new listTeacher;
    data.seekg(32 + sizeof(listStudent));
    data.read((char *)lt, sizeof(listTeacher));
    if (lt->size == 0)
    {
        printf("list teachers is empty\n");
    }
    else
    {
        printf("id: ");
        char id[8];
        cin >> id;
        person *p = new person;
        uint32_t pos = lt->head;
        while (pos != 0)
        {
            data.seekg(pos);
            data.read((char *)p, sizeof(person));
            if (strcmp(id, p->id) == 0)
            {
                break;
            }
            pos = p->nextNode;
        }
        if (pos == 0)
        {
            printf("id is not exist\n");
        }
        else
        {
            printf("name: ");
            cin.ignore();
            cin.getline(p->name, 32);
            int temp;
            printf("birthday: \n");
            printf("    day: ");
            cin >> temp;
            p->birthday.day = temp;
            printf("    month: ");
            cin >> temp;
            p->birthday.month = temp;
            printf("    year: ");
            cin >> temp;
            p->birthday.year = temp;
            printf("join date: \n");
            printf("    day: ");
            cin >> temp;
            p->join_date.day = temp;
            printf("    month: ");
            cin >> temp;
            p->join_date.month = temp;
            printf("    year: ");
            cin >> temp;
            p->join_date.year = temp;
            printf("phone: ");
            cin.ignore();
            string phone;
            getline(cin, phone);
            printf("cccd: ");
            string cccd;
            getline(cin, cccd);
            // encrypt phone and cccd
            strcpy(p->phone, crypto.encrypt(phone));
            strcpy(p->cccd, crypto.encrypt(cccd));
            data.seekg(pos);
            data.write((char *)p, sizeof(person));
            printf("edit success\n");
        }
        delete p;
    }
    delete lt;
}

void case9(fstream &data)
{
    printf("trash bin:\n- - - - - - - - - - - - - -\n");
    uint32_t sizeFile;
    data.seekg(0, std::ios::end);
    sizeFile = data.tellg();
    if (sizeFile == LENGTH_HEADER)
    {
        printf("trash bin is empty\n");
        return;
    }
    data.seekg(LENGTH_HEADER);
    person *p = new person;
    while (true)
    {
        data.read((char *)p, sizeof(person));
        if (p->role < 0)
        {
            if(p->role == -1)
            {
                printf("role: teacher\n");
            }
            else
            {
                printf("role: student\n");
            }
            printf("id: %s\n", p->id);
            printf("name: %s\n", p->name);
            printf("birthday: %d/%d/%d\n", p->birthday.day, p->birthday.month, p->birthday.year);
            printf("join date: %d/%d/%d\n", p->join_date.day, p->join_date.month, p->join_date.year);
            printf("phone: %s\n", crypto.decrypt((char *)p->phone).c_str());
            printf("cccd: %s\n", crypto.decrypt((char *)p->cccd).c_str());
            printf("\n");
        }
        if (((uint32_t)data.tellg()) + sizeof(person) > sizeFile)
        {
            break;
        }
    }
}

int main()
{
    fstream data("data.bin", std::ios::in | std::ios::out | std::ios::binary);
    initDataBase(data);
    printf("welcome to student management system\n");
    login(data);
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
        printf("9. search trash bin\n");
        printf("0. exit\n");
        printf("\nselect: ");
        int select;
        cin >> select;
        system("cls");
        switch (select)
        {
        case 1:
        {
            case1(data);
            break;
        }
        case 2:
        {
            case2(data);
            break;
        }
        case 3:
        {
            case3(data);
            break;
        }
        case 4:
        {
            case4(data);
            break;
        }
        case 5:
        {
            case5(data);
            break;
        }
        case 6:
        {
            case6(data);
            break;
        }
        case 7:
        {
            case7(data);
            break;
        }
        case 8:
        {
            case8(data);
            break;
        }
        case 9:
        {
            case9(data);
            break;
        }
        default:
        {
            data.close();
            return 0;
        }
        }
        printf("\n");
        system("pause");
        system("cls");
    }
}   