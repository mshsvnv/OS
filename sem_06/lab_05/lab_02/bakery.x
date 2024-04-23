struct BAKERY
{
    int number;
    int id;
    int letter;
};

program BAKERY_PROG
{
    version BAKERY_VER
    {
        struct BAKERY GET_NUMBER(struct BAKERY) = 1; 
        struct BAKERY WAIT_SERVICE(struct BAKERY) = 2;
        struct BAKERY GET_SERVICE(struct BAKERY) = 3;
    } = 1; /* Version number = 1 */
} = 0x20000001;
