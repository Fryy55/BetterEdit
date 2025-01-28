// @ts-check

const objs = editor.getSelectedObjects();
print(`selected objects: ${objs.length}`);
for (const obj of objs) {
    print(`id: ${obj.id}`);
}
