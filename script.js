// =========================================================
// Zafran Pakistani Kitchen — Simple Beginner JavaScript
// Handles + Add-to-Cart cards, 5% tax & C++/SQLite API calls
// =========================================================

var defaultMenu = [
    { id: 1,  name: "Chicken Biryani",     category: "From the kitchen", description: "Long-grain basmati layered with saffron and bone-in chicken.", price: 380 },
    { id: 2,  name: "Beef Karahi",         category: "From the kitchen", description: "Half a kilo of beef in tomato, ginger and black pepper.", price: 950 },
    { id: 3,  name: "Chicken Tikka",       category: "To begin",         description: "Yoghurt and red chilli marinade, charred on skewers.", price: 520 },
    { id: 4,  name: "Seekh Kebab (4 pcs)", category: "To begin",         description: "Hand-minced beef, green chilli, fresh coriander.", price: 420 },
    { id: 5,  name: "Mutton Pulao",        category: "From the kitchen", description: "Slow-braised mutton, yakhni rice, fried shallots.", price: 680 },
    { id: 6,  name: "Daal Makhani",        category: "From the kitchen", description: "Black lentils simmered overnight with butter and cream.", price: 320 },
    { id: 7,  name: "Garlic Naan",         category: "Breads & sides",   description: "Tandoor naan brushed with garlic butter and coriander.", price: 90 },
    { id: 8,  name: "Fresh Salad",         category: "To begin",         description: "Cucumber, tomato, onion, lemon and chaat masala.", price: 120 },
    { id: 9,  name: "Kheer",               category: "Sweet & cold",     description: "Rice pudding with cardamom, pistachio and rose.", price: 180 },
    { id: 10, name: "Soft Drink",          category: "Sweet & cold",     description: "Chilled bottle, regular or diet.", price: 100 }
];

// Dish pictures (saved in the images/ folder), matched by dish id
var dishImages = {
    1: "images/chicken-biryani.jpg",
    2: "images/beef-karahi.jpg",
    3: "images/chicken-tikka.jpg",
    4: "images/seekh-kebab.jpg",
    5: "images/mutton-pulao.jpg",
    6: "images/daal-makhani.jpg",
    7: "images/garlic-naan.jpg",
    8: "images/fresh-salad.jpg",
    9: "images/kheer.jpg",
    10: "images/soft-drink.jpg"
};

var menuList = defaultMenu.slice();
var activeFilter = "all";

// ---------- Simple Cart Functions ----------
function getCart() {
    var raw = localStorage.getItem("zafran_cart_v2");
    return raw ? JSON.parse(raw) : [];
}

function saveCart(cart) {
    localStorage.setItem("zafran_cart_v2", JSON.stringify(cart));
    updateHeaderCount();
}

function updateHeaderCount() {
    var cart = getCart();
    var totalItems = 0;
    for (var i = 0; i < cart.length; i++) {
        totalItems += cart[i].qty;
    }
    var badge = document.getElementById("navCartBadge");
    if (badge) {
        badge.textContent = totalItems;
        badge.hidden = (totalItems === 0);
    }
}

function getQtyInCart(id) {
    var cart = getCart();
    for (var i = 0; i < cart.length; i++) {
        if (cart[i].id === id) return cart[i].qty;
    }
    return 0;
}

function addOrUpdateCart(id, name, price, change) {
    var cart = getCart();
    var found = false;

    for (var i = 0; i < cart.length; i++) {
        if (cart[i].id === id) {
            cart[i].qty += change;
            if (cart[i].qty <= 0) {
                cart.splice(i, 1);
            }
            found = true;
            break;
        }
    }

    if (!found && change > 0) {
        cart.push({ id: id, name: name, price: price, qty: change });
    }

    saveCart(cart);
    if (document.getElementById("menuCardsGrid")) {
        renderMenuCards();
        renderSidebarCart();
    }
    if (document.getElementById("checkoutTableBody")) {
        renderCheckoutPage();
    }
}

function formatRs(val) {
    if (Math.floor(val) === val) return val.toLocaleString("en-PK");
    return val.toFixed(2);
}

// ---------- Menu Page (2-Col Cards + Plus Button + Right Cart) ----------
function renderMenuCards() {
    var grid = document.getElementById("menuCardsGrid");
    var heading = document.getElementById("menuSectionTitle");
    if (!grid) return;

    grid.innerHTML = "";
    var itemsToShow = [];

    if (activeFilter === "price_sort") {
        heading.textContent = "All dishes — Sorted low to high (Bubble Sort)";
        itemsToShow = menuList.slice();
        // Simple Bubble Sort by price
        for (var i = 0; i < itemsToShow.length - 1; i++) {
            for (var j = 0; j < itemsToShow.length - 1 - i; j++) {
                if (itemsToShow[j].price > itemsToShow[j + 1].price) {
                    var temp = itemsToShow[j];
                    itemsToShow[j] = itemsToShow[j + 1];
                    itemsToShow[j + 1] = temp;
                }
            }
        }
    } else if (activeFilter === "all") {
        heading.textContent = "All dishes";
        itemsToShow = menuList;
    } else {
        heading.textContent = activeFilter;
        for (var k = 0; k < menuList.length; k++) {
            if (menuList[k].category === activeFilter) {
                itemsToShow.push(menuList[k]);
            }
        }
    }

    for (var m = 0; m < itemsToShow.length; m++) {
        (function (dish) {
            var card = document.createElement("article");
            card.className = "menu-item-card";

            var qty = getQtyInCart(dish.id);
            var btnLabel = qty > 0 ? qty : "+";
            var btnClass = qty > 0 ? "plus-btn in-cart" : "plus-btn";

            var imgSrc = dishImages[dish.id] || "images/chicken-biryani.jpg";

            card.innerHTML =
                "<img class='menu-item-img' src='" + imgSrc + "' alt='" + dish.name + "' width='600' height='420' loading='lazy'>" +
                "<div>" +
                    "<h3>" + dish.name + "</h3>" +
                    "<div class='menu-item-price'>Rs " + dish.price + "</div>" +
                    "<p class='menu-item-desc'>" + dish.description + "</p>" +
                "</div>" +
                "<div class='card-bottom-row'>" +
                    "<span class='cat-badge'>" + dish.category + "</span>" +
                    "<button type='button' class='" + btnClass + "' aria-label='Add " + dish.name + " to order'>" + btnLabel + "</button>" +
                "</div>";

            card.querySelector("button").addEventListener("click", function () {
                addOrUpdateCart(dish.id, dish.name, dish.price, 1);
            });

            grid.appendChild(card);
        })(itemsToShow[m]);
    }
}

function renderSidebarCart() {
    var box = document.getElementById("sidebarCartItems");
    var totBox = document.getElementById("sidebarCartTotals");
    if (!box || !totBox) return;

    var cart = getCart();
    if (cart.length === 0) {
        box.innerHTML = "<p style='color:#6e655d; font-size:0.9rem; padding:18px 0;'>Your order is empty. Click <strong>+</strong> on any dish card to add it here.</p>";
        totBox.hidden = true;
        return;
    }

    box.innerHTML = "";
    var subtotal = 0;

    for (var i = 0; i < cart.length; i++) {
        (function (item) {
            var lineAmt = item.price * item.qty;
            subtotal += lineAmt;

            var row = document.createElement("div");
            row.className = "cart-item-row";
            row.innerHTML =
                "<div><strong style='font-size:0.94rem;'>" + item.name + "</strong><br><small class='mono' style='color:#6e655d;'>Rs " + formatRs(lineAmt) + "</small></div>" +
                "<div class='cart-stepper'>" +
                    "<button type='button' class='step-btn minus' aria-label='One less " + item.name + "'>&minus;</button>" +
                    "<strong class='mono' style='font-size:0.88rem;'>" + item.qty + "</strong>" +
                    "<button type='button' class='step-btn plus' aria-label='One more " + item.name + "'>+</button>" +
                "</div>";

            row.querySelector(".minus").addEventListener("click", function () {
                addOrUpdateCart(item.id, item.name, item.price, -1);
            });
            row.querySelector(".plus").addEventListener("click", function () {
                addOrUpdateCart(item.id, item.name, item.price, 1);
            });

            box.appendChild(row);
        })(cart[i]);
    }

    var tax = subtotal * 0.05;
    var total = subtotal + tax;

    document.getElementById("sideSubtotal").textContent = "Rs " + formatRs(subtotal);
    document.getElementById("sideTax").textContent = "Rs " + formatRs(tax);
    document.getElementById("sideTotal").textContent = "Rs " + formatRs(total);
    totBox.hidden = false;
}

// ---------- Order / Checkout Page ----------
function renderCheckoutPage() {
    var tbody = document.getElementById("checkoutTableBody");
    if (!tbody) return;

    var cart = getCart();
    var emptySec = document.getElementById("emptyCartSection");
    var mainSec = document.getElementById("checkoutMainSection");
    var rcptSec = document.getElementById("receiptOutputSection");

    if (cart.length === 0) {
        if (rcptSec && !rcptSec.hidden) return;
        emptySec.hidden = false;
        mainSec.hidden = true;
        return;
    }

    emptySec.hidden = true;
    mainSec.hidden = false;
    tbody.innerHTML = "";

    var subtotal = 0;
    for (var i = 0; i < cart.length; i++) {
        (function (item) {
            var amt = item.price * item.qty;
            subtotal += amt;

            var tr = document.createElement("tr");
            tr.innerHTML =
                "<th scope='row'>" + item.name + "<br><small class='mono' style='font-weight:normal;color:#6e655d;'>Rs " + item.price + " each</small></th>" +
                "<td><div class='cart-stepper'>" +
                    "<button type='button' class='step-btn minus' aria-label='One less " + item.name + "'>&minus;</button>" +
                    "<strong class='mono' style='font-size:0.88rem;'>" + item.qty + "</strong>" +
                    "<button type='button' class='step-btn plus' aria-label='One more " + item.name + "'>+</button>" +
                "</div></td>" +
                "<td class='mono' style='text-align:right;'>Rs " + formatRs(amt) + "</td>";

            tr.querySelector(".minus").addEventListener("click", function () {
                addOrUpdateCart(item.id, item.name, item.price, -1);
            });
            tr.querySelector(".plus").addEventListener("click", function () {
                addOrUpdateCart(item.id, item.name, item.price, 1);
            });
            tbody.appendChild(tr);
        })(cart[i]);
    }

    var tax = subtotal * 0.05;
    var total = subtotal + tax;
    document.getElementById("chkSubtotal").textContent = "Rs " + formatRs(subtotal);
    document.getElementById("chkTax").textContent = "Rs " + formatRs(tax);
    document.getElementById("chkTotal").textContent = "Rs " + formatRs(total);
    var btnTot = document.getElementById("btnOrderTotal");
    if (btnTot) btnTot.textContent = "Rs " + formatRs(total);
}

function setupCheckoutForm() {
    var form = document.getElementById("checkoutForm");
    if (!form) return;

    var savedMode = localStorage.getItem("zafran_service_mode");
    var rDine = document.getElementById("chkDineIn");
    var rPick = document.getElementById("chkPickup");
    var tblBox = document.getElementById("tableInputBox");

    if (savedMode === "Pickup") {
        rPick.checked = true;
        tblBox.hidden = true;
    }

    rDine.addEventListener("change", function () { tblBox.hidden = false; });
    rPick.addEventListener("change", function () { tblBox.hidden = true; });

    var nameIn = document.getElementById("custName");
    var phoneIn = document.getElementById("custPhone");
    var errName = document.getElementById("errName");
    var errPhone = document.getElementById("errPhone");

    nameIn.addEventListener("input", function () { nameIn.removeAttribute("aria-invalid"); errName.textContent = ""; });
    phoneIn.addEventListener("input", function () { phoneIn.removeAttribute("aria-invalid"); errPhone.textContent = ""; });

    form.addEventListener("submit", function (e) {
        e.preventDefault();

        if (nameIn.value.trim().length < 2) {
            nameIn.setAttribute("aria-invalid", "true");
            errName.textContent = "Please enter customer name.";
            nameIn.focus();
            return;
        }
        if (!phoneIn.checkValidity() || phoneIn.value.trim() === "") {
            phoneIn.setAttribute("aria-invalid", "true");
            errPhone.textContent = "Please enter a valid phone number.";
            phoneIn.focus();
            return;
        }

        var cart = getCart();
        if (cart.length === 0) return;

        var subtotal = 0;
        var summaryList = [];
        var formPairs = [];
        for (var i = 0; i < cart.length; i++) {
            subtotal += cart[i].price * cart[i].qty;
            summaryList.push(cart[i].qty + " x " + cart[i].name);
            formPairs.push("qty_" + cart[i].id + "=" + encodeURIComponent(cart[i].qty));
        }

        var service = rDine.checked ? "Dine in" : "Pickup";
        var tbl = document.getElementById("custTable").value.trim();
        var notes = document.getElementById("custNotes").value.trim();

        formPairs.push("guest_name=" + encodeURIComponent(nameIn.value.trim()));
        formPairs.push("phone=" + encodeURIComponent(phoneIn.value.trim()));
        formPairs.push("service=" + encodeURIComponent(service));
        formPairs.push("table_no=" + encodeURIComponent(tbl));
        formPairs.push("notes=" + encodeURIComponent(notes));

        fetch("/api/order", {
            method: "POST",
            headers: { "Content-Type": "application/x-www-form-urlencoded" },
            body: formPairs.join("&")
        })
        .then(function (r) { return r.json(); })
        .then(function (res) {
            displayReceipt(cart, nameIn.value.trim(), phoneIn.value.trim(), service, tbl, notes, subtotal, res.receipt_no, res.created_at);
        })
        .catch(function () {
            var rno = "ZF-261125-" + Math.floor(1000 + Math.random() * 8999);
            var dt = new Date().toLocaleString();
            var localOrders = JSON.parse(localStorage.getItem("zafran_orders_v2") || "[]");
            localOrders.unshift({
                receipt_no: rno,
                customer_name: nameIn.value.trim(),
                service_type: (service === "Dine in" && tbl) ? ("Dine in (Table " + tbl + ")") : service,
                items_summary: summaryList.join(", "),
                total: subtotal * 1.05,
                created_at: dt
            });
            localStorage.setItem("zafran_orders_v2", JSON.stringify(localOrders));
            displayReceipt(cart, nameIn.value.trim(), phoneIn.value.trim(), service, tbl, notes, subtotal, rno, dt);
        });
    });

    var btnPrint = document.getElementById("btnPrint");
    if (btnPrint) btnPrint.addEventListener("click", function () { window.print(); });
}

function displayReceipt(cart, guest, phone, service, tbl, notes, subtotal, rno, dt) {
    saveCart([]);
    document.getElementById("checkoutMainSection").hidden = true;
    document.getElementById("emptyCartSection").hidden = true;
    document.getElementById("receiptOutputSection").hidden = false;

    document.getElementById("rNo").textContent = rno;
    document.getElementById("rDate").textContent = dt;
    document.getElementById("rGuest").textContent = guest;
    document.getElementById("rPhone").textContent = phone;
    document.getElementById("rService").textContent = (service === "Dine in" && tbl) ? ("Dine in · Table " + tbl) : service;

    var itemsBox = document.getElementById("rItems");
    itemsBox.innerHTML = "";
    for (var i = 0; i < cart.length; i++) {
        var line = document.createElement("div");
        line.style.display = "flex";
        line.style.justifyContent = "space-between";
        line.style.borderBottom = "1px dashed #e3dcd2";
        line.style.padding = "10px 0";
        line.innerHTML =
            "<div><strong>" + cart[i].name + "</strong><br><small class='mono' style='color:#6e655d;'>" + cart[i].qty + " &times; Rs " + cart[i].price + "</small></div>" +
            "<span class='mono' style='font-weight:500;'>Rs " + formatRs(cart[i].price * cart[i].qty) + "</span>";
        itemsBox.appendChild(line);
    }

    var tax = subtotal * 0.05;
    document.getElementById("rSub").textContent = "Rs " + formatRs(subtotal);
    document.getElementById("rTax").textContent = "Rs " + formatRs(tax);
    document.getElementById("rTot").textContent = "Rs " + formatRs(subtotal + tax);

    var nBox = document.getElementById("rNotesBox");
    var nEl = document.getElementById("rNotes");
    if (notes) {
        nBox.hidden = false;
        nEl.textContent = notes;
    } else {
        nBox.hidden = true;
    }
}

// ---------- Visit Page: Dynamic Saved Orders (Hidden when 0 orders!) ----------
function loadDynamicSavedOrders() {
    var sec = document.getElementById("savedOrdersSection");
    var tbody = document.getElementById("savedOrdersTbody");
    if (!sec || !tbody) return;

    fetch("/api/orders")
        .then(function (r) { return r.json(); })
        .then(function (rows) { renderOrdersIfAny(rows); })
        .catch(function () {
            var localRows = JSON.parse(localStorage.getItem("zafran_orders_v2") || "[]");
            renderOrdersIfAny(localRows);
        });
}

function renderOrdersIfAny(rows) {
    var sec = document.getElementById("savedOrdersSection");
    var tbody = document.getElementById("savedOrdersTbody");

    // Only show the Saved Receipts section once at least 1 order has been placed!
    if (!rows || rows.length === 0) {
        sec.hidden = true;
        return;
    }

    sec.hidden = false;
    tbody.innerHTML = "";
    var grandSales = 0;

    for (var i = 0; i < rows.length; i++) {
        var r = rows[i];
        grandSales += Number(r.total || 0);
        var tr = document.createElement("tr");
        tr.innerHTML =
            "<th scope='row' class='mono'>" + r.receipt_no + "</th>" +
            "<td>" + r.customer_name + "</td>" +
            "<td>" + r.service_type + "</td>" +
            "<td>" + r.items_summary + "</td>" +
            "<td class='mono' style='font-weight:600;'>Rs " + formatRs(Number(r.total)) + "</td>" +
            "<td>" + r.created_at + "</td>";
        tbody.appendChild(tr);
    }

    document.getElementById("totalOrderCount").textContent = rows.length;
    document.getElementById("totalSalesAmount").textContent = "Rs " + formatRs(grandSales);
}

// ---------- Initialize Current Page ----------
document.addEventListener("DOMContentLoaded", function () {
    updateHeaderCount();

    var params = new URLSearchParams(window.location.search);
    if (params.get("cat")) activeFilter = params.get("cat");
    if (params.get("sort") === "price") activeFilter = "price_sort";

    var tabs = document.querySelectorAll("#categoryTabs .tab-btn");
    for (var t = 0; t < tabs.length; t++) {
        if (tabs[t].getAttribute("data-filter") === activeFilter) {
            for (var x = 0; x < tabs.length; x++) tabs[x].classList.remove("active");
            tabs[t].classList.add("active");
        }
        tabs[t].addEventListener("click", function () {
            for (var x = 0; x < tabs.length; x++) tabs[x].classList.remove("active");
            this.classList.add("active");
            activeFilter = this.getAttribute("data-filter");
            renderMenuCards();
        });
    }

    var sideDine = document.getElementById("sideDineIn");
    var sidePick = document.getElementById("sidePickup");
    if (sideDine && sidePick) {
        var saved = localStorage.getItem("zafran_service_mode");
        if (saved === "Pickup") sidePick.checked = true;
        sideDine.addEventListener("change", function () { localStorage.setItem("zafran_service_mode", "Dine in"); });
        sidePick.addEventListener("change", function () { localStorage.setItem("zafran_service_mode", "Pickup"); });
    }

    if (document.getElementById("menuCardsGrid")) {
        fetch("/api/menu")
            .then(function (r) { return r.json(); })
            .then(function (data) {
                if (data && data.length > 0) menuList = data;
                renderMenuCards();
                renderSidebarCart();
            })
            .catch(function () {
                renderMenuCards();
                renderSidebarCart();
            });
    }

    if (document.getElementById("checkoutTableBody")) {
        renderCheckoutPage();
        setupCheckoutForm();
    }

    if (document.getElementById("savedOrdersSection")) {
        loadDynamicSavedOrders();
        setInterval(loadDynamicSavedOrders, 4000);
    }
});
