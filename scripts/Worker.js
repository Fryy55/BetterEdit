
function deselectRandom(objs) {
    for (obj of objs) {
        if (Math.random() > 0.5) {
            obj.deselect();
        }
    }
}

// This is just an idea
editor.addEventListener("select", objs => {
    deselectRandom(objs);
});
