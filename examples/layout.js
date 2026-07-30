for (let i = 0; i < 4; i++) {
    for (let j = 0; j < 8; j++) {
        new LED(10 - (j-7)*35, 10 + (i*60), IrqManager.getPinIrq(i, j), "rgb(60,169,213)")
    }
}
new PrintPort(10, 250, IrqManager.getIrqFromName("PB"));