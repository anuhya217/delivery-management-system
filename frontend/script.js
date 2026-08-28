const API_URL = "http://127.0.0.1:18080/api/orders";

async function loadOrders() {

    console.log("Loading orders from:", API_URL);

    try {

        const response = await fetch(API_URL);

        console.log("Response status:", response.status);

        if (!response.ok) {
            throw new Error("API returned status " + response.status);
        }

        const data = await response.json();

        console.log("API Data:", data);

        const orders = data.orders || [];

        // Total Orders
        document.getElementById("totalOrders").textContent =
            orders.length;


        // Delivered Orders
        const delivered = orders.filter(order =>
            order.status === "delivered"
        ).length;

        document.getElementById("deliveredOrders").textContent =
            delivered;


        // Pending Orders
        const pending = orders.filter(order =>
            order.status !== "delivered"
        ).length;

        document.getElementById("pendingOrders").textContent =
            pending;


        // Active Drivers
        const activeDrivers = new Set(
            orders
                .filter(order => order.driver_id)
                .map(order => order.driver_id)
        );

        const activeDriverElement =
            document.getElementById("activeDrivers");

        if (activeDriverElement) {
            activeDriverElement.textContent =
                activeDrivers.size;
        }


        // Orders Table
        const table = document.getElementById("ordersTable");

        table.innerHTML = "";


        orders.forEach(order => {

            const row = document.createElement("tr");

            row.innerHTML = `
                <td>${order.id}</td>
                <td>${order.dsp_id}</td>
                <td>${order.driver_id || "Not Assigned"}</td>
                <td>${order.pickup_address}</td>
                <td>${order.delivery_address}</td>
                <td>
                    <span class="status">
                        ${order.status}
                    </span>
                </td>
                <td>${order.sla_deadline}</td>
            `;

            table.appendChild(row);

        });


        // If no orders
        if (orders.length === 0) {

            table.innerHTML = `
                <tr>
                    <td colspan="7" style="text-align:center;">
                        No orders found
                    </td>
                </tr>
            `;

        }

    } catch (error) {

        console.error("Error loading orders:", error);

        alert("Cannot connect to Delivery API! Check Console (F12)");

    }

}

loadOrders();